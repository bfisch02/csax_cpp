#include <algorithm>
#include <iostream>

#include "sampletransformer.hpp"
#include "vectormath.hpp"
#include "errormodel.hpp"

void FeatureFilter::transformSampleInPlace(Sample& s){
  unsigned contvarDropCount = 0;
  for(unsigned i = 0; i < ss.contvarCount; i++){
    if(dropContvar(i)){
      i++;
      contvarDropCount++;
      if(i == ss.contvarCount) break;
    }
    s.contvars[i - contvarDropCount] = s.contvars[i];
  }

  unsigned catvarDropCount = 0;
  for(unsigned i = 0; i < ss.catvarCount; i++){
    if(dropCatvar(i)){
      i++;
      catvarDropCount++;
      if(i == ss.catvarCount) break;
    }
    s.catvars[i - catvarDropCount] = s.catvars[i];
  }

  unsigned binvarDropCount = 0;
  for(unsigned i = 0; i < ss.binvarCount; i++){
    if(dropBinvar(i)){
      i++;
      binvarDropCount++;
      if(i == ss.binvarCount) break;
    }
    s.binvars[i - binvarDropCount] = s.binvars[i];
  }
}

void FeatureFilter::transformSampleStructureInPlace(SampleStructure& ss){
  unsigned contvarDropCount = 0;
  for(unsigned i = 0; i < ss.contvarCount; i++){
    if(dropContvar(i)){
      contvarDropCount++;
      if(i == ss.featureCount() - 1) break; //Done (last feature was dropped)
    }
    ss.nameLookup[i + 1 - contvarDropCount] = ss.nameLookup[i + 1];
    if(i < ss.contvarCount - 1 && ss.contvarMeans) ss.contvarMeans[i + 1 - contvarDropCount] = ss.contvarMeans[i + 1]; //Optimization: This is slow.
  }

  unsigned catvarDropCount = 0;
  for(unsigned i = 0; i < ss.catvarCount; i++){
    if(dropCatvar(i)){
      catvarDropCount++;
      if(i + ss.contvarCount == ss.featureCount() - 1) break; //Done (last feature was dropped)
    }
    ss.nameLookup[i + 1 + ss.contvarCount - contvarDropCount - catvarDropCount] = ss.nameLookup[i + 1 + ss.contvarCount];
    
    if(i < ss.catvarCount - 1){
      ss.catvarCatNames[i + 1 - catvarDropCount] = ss.catvarCatNames[i + 1];
      ss.catvarSizes[i + 1 - catvarDropCount] = ss.catvarSizes[i + 1];
    }
  }
  

  unsigned binvarDropCount = 0;
  for(unsigned i = 0; i < ss.binvarCount; i++){
    if(dropBinvar(i)){
      binvarDropCount++;
      if(i + ss.contvarCount + ss.binvarCount == ss.featureCount() - 1) break; //Done (last feature was dropped)
    }
    ss.nameLookup[i + 1 + ss.contvarCount - contvarDropCount + ss.catvarCount - catvarDropCount - binvarDropCount] = ss.nameLookup[i + 1 + ss.contvarCount + ss.catvarCount];
    if(i < ss.binvarCount - 1){
      ss.binvarCatNames[(i + 1 - binvarDropCount) * 2 + 0] = ss.binvarCatNames[(i + 1) * 2 + 0];
      ss.binvarCatNames[(i + 1 - binvarDropCount) * 2 + 1] = ss.binvarCatNames[(i + 1) * 2 + 1];
    }
  }

  ss.contvarCount -= contvarDropCount;
  ss.catvarCount -= catvarDropCount;
  ss.binvarCount -= binvarDropCount;
}


//Need this to order some stuff.
struct IndexFloatPair{
  fracfloat_t f;
  unsigned i;
  
  IndexFloatPair(){};
  IndexFloatPair(fracfloat_t f, unsigned i) : f(f), i(i){}
};

bool operator<(IndexFloatPair p0, IndexFloatPair p1){
  return p0.f < p1.f;
}

//Filter helpers
static void scoreFeatureSetCont(SampleStructure ss, IndexFloatPair* out, std::function<void(IndexFloatPair*, unsigned)> f){
  
  //Optimization: more sloppy parallelism, use a counting iterator.
  unsigned featureIndices [ss.contvarCount];
  for(unsigned i = 0; i < ss.contvarCount; i++){
    featureIndices[i] = i;
    out[i].i = i + 0;
  }
  
  std::for_each(featureIndices, featureIndices + ss.contvarCount, [&](unsigned i){f(out, i);});
}
static void scoreFeatureSetCat(SampleStructure ss, IndexFloatPair* out, std::function<void(IndexFloatPair*, unsigned)> f){
  
  //Optimization: more sloppy parallelism, use a counting iterator.
  unsigned featureIndices [ss.catvarCount];
  for(unsigned i = 0; i < ss.catvarCount; i++){
    featureIndices[i] = i;
    out[i].i = i + ss.contvarCount;
  }
  
  std::for_each(featureIndices, featureIndices + ss.catvarCount, [&](unsigned i){f(out, i);});
}
static void scoreFeatureSetBin(SampleStructure ss, IndexFloatPair* out, std::function<void(IndexFloatPair*, unsigned)> f){
  
  //Optimization: more sloppy parallelism, use a counting iterator.
  unsigned featureIndices [ss.binvarCount];
  for(unsigned i = 0; i < ss.binvarCount; i++){
    featureIndices[i] = i;
    out[i].i = i + ss.contvarCount + ss.catvarCount;
  }
  
  std::for_each(featureIndices, featureIndices + ss.binvarCount, [&](unsigned i){f(out, i);});
}

static void scoreFeatureSet(SampleStructure ss, IndexFloatPair* out, std::function<void(IndexFloatPair*, unsigned)> f_cont, std::function<void(IndexFloatPair*, unsigned)> f_cat, std::function<void(IndexFloatPair*, unsigned)> f_bin){
  //Parallelism: These three could be done in parallel.
  scoreFeatureSetCont(ss, out, f_cont);
  scoreFeatureSetCat (ss, out + ss.contvarCount, f_cat);
  scoreFeatureSetBin (ss, out + ss.contvarCount + ss.catvarCount, f_bin);
}

BitArray makeFilter(SampleStructure ss, fracfloat_t filterPercentile, bool verbose, std::string filterCriterionName, std::function<void(IndexFloatPair*, unsigned)> f_cont, std::function<void(IndexFloatPair*, unsigned)> f_cat, std::function<void(IndexFloatPair*, unsigned)> f_bin){

  unsigned topRankToDrop = (unsigned)(ss.featureCount() * filterPercentile);
  
  IndexFloatPair out[ss.featureCount()];
  
  scoreFeatureSet(ss, out, f_cont, f_cat, f_bin);
  
  //Make sure indices were properly set.
  for(unsigned i = 0; i < ss.featureCount(); i++){
    assert(out[i].i == i);
  }
  

  if(verbose){
    //Use partial sort for ordered output
    std::partial_sort(out, out + topRankToDrop, out + ss.featureCount());
  }
  else{
    //Use faster kth order statistics
    //Optimization: Not sure if this is median of medians or randomized version.
    std::nth_element(out, out + topRankToDrop, out + ss.featureCount()); //First, Middle, Last
  }
  BitArray filter(ss.featureCount());
  
  for(unsigned i = 0; i < topRankToDrop; i++){
    filter[out[i].i] = true;
    if(verbose)std::cerr << "Dropping Feature " << out[i].i << " \"" << ss.featureName(out[i].i) << "\", " << filterCriterionName << " = " << out[i].f << std::endl;
  }
  
  return filter;
}

RealDist* approximateDistributionOfRealFeature(Array<Sample> s, unsigned index){
  Array<fracfloat_t> values = Array<fracfloat_t>(s.length);
  #ifdef UNKNOWNVALUES
  //Do some filtering
  unsigned outIndex = 0;
  for(unsigned i = 0; i < values.length; i++){
    if(s[i].contvarUnknown(index)) continue;
    values[outIndex++] = s[i].contvars[index];
  }
  values.length = outIndex;
  #else
  for(unsigned i = 0; i < values.length; i++){
    values[i] = s[i].contvars[index];
  }
  #endif
  
  MultiGaussianWeighted* dist = new MultiGaussianWeighted();
  *dist = MultiGaussianWeighted::fitGaussianKernel(values.data, values.length, (unsigned)sqrt(values.length));
  
  return dist;
}

fracfloat_t klReal(RealDist* p, RealDist* q){

  //TODO: This is a terrible integral.
  //Heuristic so we're taking the integral of the rightish area.
  fracfloat_t a = p->argMax();
  fracfloat_t b = q->argMax();
  
  fracfloat_t minVal = min2<fracfloat_t>(a, b) - 10;
  fracfloat_t maxVal = max2<fracfloat_t>(a, b) + 10;
  
  unsigned granularity = 256;
  fracfloat_t width = (maxVal - minVal) / granularity;

  //LRAM approximation
  
  fracfloat_t value = 0;
  for(unsigned i = 0; i < granularity; i++){
    fracfloat_t x = minVal + (i * width);
    value += p->likelihood(x) * (p->logLikelihood(x) - q->logLikelihood(x)); //Optimization: redundant p query.
  }
  return value * width;
}

void count(Array<Sample> s, unsigned index, fracfloat_t* out){   
  for(unsigned j = 0; j < s.length; j++){
    #ifdef UNKNOWNVALUES
    if(s[j].catvarUnknown(index)) continue;
    #endif
    out[s[j].catvars[index]] += 1;
  }
}

void distributionOfCatFeature(Array<Sample> s, unsigned index, fracfloat_t* out, unsigned size){
    //Laplace smooth the distribution
    arraySet<fracfloat_t>(out, size, 1);

    count(s, index, out);
    
    //Normalize the distribution
    normalizeVectorSumToOneInPlace<fracfloat_t>(out, size);
}


BitArray klDivergenceFilterBools(SampleStructure ss, fracfloat_t filterPercentile, Array<Sample> training, Array<Sample> test, bool verbose){
  std::function<void(IndexFloatPair*, unsigned)> contFilter = [&](IndexFloatPair* out, unsigned i){
    RealDist* pDist = approximateDistributionOfRealFeature(training, i);
    RealDist* qDist = approximateDistributionOfRealFeature(test, i);
    
    out[i].f = klReal(pDist, qDist);
    
    //Clean up some memory
    pDist->freeMemory();
    qDist->freeMemory();
    delete pDist;
    delete qDist;
  };
  
  std::function<void(IndexFloatPair*, unsigned)> catFilter = [&](IndexFloatPair* out, unsigned i){
    fracfloat_t pDist [ss.catvarSizes[i]];
    fracfloat_t qDist [ss.catvarSizes[i]];

    distributionOfCatFeature(training, i, pDist, ss.catvarSizes[i]);
    distributionOfCatFeature(test, i, qDist, ss.catvarSizes[i]);
    
    out[i].f = klDivergenceStrictPositive(pDist, qDist, ss.catvarSizes[i]);
  };
  
  //TODO bin values not supported
  std::function<void(IndexFloatPair*, unsigned)> binFilter = [&](IndexFloatPair* out, unsigned i){
    out[i].f = 0;
    assert(0);
  };
  
  return makeFilter(ss, filterPercentile, verbose, "KL Divergence", contFilter, catFilter, binFilter);
}
BitArray entropyFilterBoolsTransductive(SampleStructure ss, fracfloat_t filterPercentile, Array<Sample> training, Array<Sample> test, bool verbose){
  
  //Copy all to a new array
  Sample data[training.length + test.length]; //Stack allocate
  Array<Sample> all = Array<Sample>(data, training.length + test.length);
  
  arrayCopy<Sample>(all.data + 0, training.data, training.length);
  arrayCopy<Sample>(all.data + training.length, test.data, test.length);
  
  return entropyFilterBools(ss, filterPercentile, all, verbose, "Entropy (transductive)");
}
BitArray entropyFilterBools(SampleStructure ss, fracfloat_t filterPercentile, Array<Sample> training, bool verbose, std::string name){

  std::function<void(IndexFloatPair*, unsigned)> contFilter = [&](IndexFloatPair* out, unsigned i){
    
    //Architecture: This is basically the same thing as the approximate dist function used for KL, but we need an intermediate value.

    Array<fracfloat_t> values = Array<fracfloat_t>(training.length);
    #ifdef UNKNOWNVALUES
    //Do some filtering
    unsigned outIndex = 0;
    for(unsigned j = 0; j < values.length; j++){
      if(training[j].contvarUnknown(i)) continue;
      values[outIndex++] = training[j].contvars[i];
    }
    values.length = outIndex;
    #else
    for(unsigned j = 0; j < values.length; j++){
      values[j] = training[j].contvars[i];
    }
    #endif

    MultiGaussianWeighted* dist = new MultiGaussianWeighted();
    *dist = MultiGaussianWeighted::fitGaussianKernel(values.data, values.length, (unsigned)sqrt(values.length));

    
    out[i].f = dist->approximateDifferentialEntropyFromSamples(values);
    
    //Clean up some memory
    dist->freeMemory();
    delete dist;
  };
  
  std::function<void(IndexFloatPair*, unsigned)> catFilter = [&](IndexFloatPair* out, unsigned i){
    fracfloat_t dist [ss.catvarSizes[i]];

    distributionOfCatFeature(training, i, dist, ss.catvarSizes[i]);
    
    out[i].f = entropyStrictPositive(dist, ss.catvarSizes[i]);
  };
  
  //TODO binary values not supported
  std::function<void(IndexFloatPair*, unsigned)> binFilter = [&](IndexFloatPair* out, unsigned i){
    out[i].f = 0;
    assert(0);
  };
  return makeFilter(ss, filterPercentile, verbose, name, contFilter, catFilter, binFilter);
}


SampleTransformer* klDivergenceFilter(SampleStructure ss, fracfloat_t filterPercentile, Array<Sample> p, Array<Sample> q, bool verbose){
  if((unsigned)(ss.featureCount() * filterPercentile) == 0) return new NullSampleTransformer();
  return new FeatureFilter(klDivergenceFilterBools(ss, filterPercentile, p, q, verbose), ss);
}

SampleTransformer* lowEntropyFilterTransductive(SampleStructure ss, fracfloat_t filterPercentile, Array<Sample> p, Array<Sample> q, bool verbose){
  if((unsigned)(ss.featureCount() * filterPercentile) == 0) return new NullSampleTransformer();
  return new FeatureFilter(entropyFilterBoolsTransductive(ss, filterPercentile, p, q, verbose), ss);
}

SampleTransformer* lowEntropyFilter(SampleStructure ss, fracfloat_t filterPercentile, Array<Sample> p, bool verbose){
  if((unsigned)(ss.featureCount() * filterPercentile) == 0) return new NullSampleTransformer();
  return new FeatureFilter(entropyFilterBools(ss, filterPercentile, p, verbose), ss);
}

SampleTransformer* randomFilter(SampleStructure ss, fracfloat_t filterPercentile, bool verbose){

  
  //Note: only does catvars
  
  //Optimization: Inefficient random selection without replacement
  
  unsigned possibilities[ss.featureCount()];
  for(unsigned i = 0; i < ss.featureCount(); i++){
    possibilities[i] = i;
  }
  
  
  std::random_device rd;
  std::default_random_engine g(rd());
  
  std::shuffle(possibilities, possibilities + ss.featureCount(), g);
  
  //Now take the first few.
  
  unsigned numToTake = (unsigned)(filterPercentile * ss.featureCount());
  
  BitArray filter(ss.featureCount());
  
  for(unsigned i = 0; i < numToTake; i++){
    filter[possibilities[i]] = true;
    if(verbose) std::cerr << "Randomly Dropping Feature " << possibilities[i] << " \"" << ss.featureName(possibilities[i]) << "\"." << std::endl;
  }
  
  return new FeatureFilter(filter, ss);
}

//Normalizers:

ZScoreAntinormalizer invertNormalizer(ZScoreNormalizer normalizer){
  return ZScoreAntinormalizer(normalizer.ss, normalizer.meanValues, normalizer.stdevValues);
}

ZScoreNormalizer invertAntinormalizer(ZScoreAntinormalizer antinormalizer){
  return ZScoreNormalizer(antinormalizer.ss, antinormalizer.meanValues, antinormalizer.stdevValues);
}
