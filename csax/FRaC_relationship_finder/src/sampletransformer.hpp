#ifndef SAMPLETRANSFORMER_H
#define	SAMPLETRANSFORMER_H

#include "sample.hpp"

class SampleTransformer {
  public:
  virtual void transformSampleInPlace(Sample& s) = 0;
  virtual void transformSampleStructureInPlace(SampleStructure& ss) { } //Empty default implementation, not all sample transformers use this.

  virtual ~SampleTransformer() { }
  
  virtual void freeMemory() { };
};

//A sample transformer that does nothing.
class NullSampleTransformer : public SampleTransformer {
  void transformSampleInPlace(Sample& s){ }
};

//A sample transformer that removes features.
class FeatureFilter : public SampleTransformer{
  BitArray drops;
  SampleStructure ss;
  
  public:
  FeatureFilter(BitArray filter, SampleStructure sampleStructure) : drops(filter), ss(sampleStructure) { }
  //Optimization: Keep track of first index in each.
  
  void transformSampleInPlace(Sample& s);
  void transformSampleStructureInPlace(SampleStructure& ss);
  
  bool dropContvar(unsigned index){
    return drops[ss.contvarIndex(index)];
  }
  bool dropCatvar(unsigned index){
    return drops[ss.catvarIndex(index)];
  }
  bool dropBinvar(unsigned index){
    return drops[ss.binvarIndex(index)];
  }
};


BitArray klDivergenceFilterBools(SampleStructure ss, fracfloat_t filterPercentile, Array<Sample> training, Array<Sample> test, bool verbose);
BitArray entropyFilterBoolsTransductive(SampleStructure ss, fracfloat_t filterPercentile, Array<Sample> training, Array<Sample> test, bool verbose);
BitArray entropyFilterBools(SampleStructure ss, fracfloat_t filterPercentile, Array<Sample> training, bool verbose, std::string name = "Entropy");


//Data dependent functions to create sample transformers (filter method techniques).
SampleTransformer* klDivergenceFilter(SampleStructure ss, fracfloat_t stripPercentile, Array<Sample> training, Array<Sample> test, bool verbose = false);
SampleTransformer* lowEntropyFilterTransductive(SampleStructure ss, fracfloat_t stripPercentile, Array<Sample> training, Array<Sample> test, bool verbose = false);
SampleTransformer* lowEntropyFilter(SampleStructure ss, fracfloat_t stripPercentile, Array<Sample> training, bool verbose = false);

SampleTransformer* randomFilter(SampleStructure ss, fracfloat_t stripPercentile, bool verbose);

//Normalizer and antinormalizer.
class ZScoreNormalizer : public SampleTransformer{
  public:
  SampleStructure ss;
  
  fracfloat_t* meanValues;
  fracfloat_t* stdevValues;
  
  ZScoreNormalizer(){};
  ZScoreNormalizer(SampleStructure ss, fracfloat_t* meanValues, fracfloat_t* stdevValues) : ss(ss), meanValues(meanValues), stdevValues(stdevValues) { };
  ZScoreNormalizer(StructuredSampleCollection ssc) : ss(ssc.sampleStructure){
      meanValues = ssc.calculateMeanContFeatures();
      stdevValues = ssc.calculateStdevContFeatures(meanValues);
  }
  void transformSampleInPlace(Sample& s){
    for(unsigned i = 0; i < ss.contvarCount; i++){
      s.contvars[i] = (s.contvars[i] - meanValues[i]) / stdevValues[i];
    }
  }
};
//Performs inverse of normalization step.
class ZScoreAntinormalizer : public SampleTransformer{
  public:
  SampleStructure ss;
  
  fracfloat_t* meanValues;
  fracfloat_t* stdevValues;
  
  ZScoreAntinormalizer(){};
  ZScoreAntinormalizer(SampleStructure ss, fracfloat_t* meanValues, fracfloat_t* stdevValues) : ss(ss), meanValues(meanValues), stdevValues(stdevValues) { };
  void transformSampleInPlace(Sample& s){
    for(unsigned i = 0; i < ss.contvarCount; i++){
      s.contvars[i] = s.contvars[i] * stdevValues[i] + meanValues[i];
    }
  }
};

//Inversion of normalizers
ZScoreAntinormalizer invertNormalizer(ZScoreNormalizer normalizer);
ZScoreNormalizer invertAntinormalizer(ZScoreAntinormalizer antinormalizer);

#endif
