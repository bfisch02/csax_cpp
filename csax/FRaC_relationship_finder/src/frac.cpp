#include "frac.hpp"
#include "errormodelcollection.hpp"

#include <math.h>
//#include <assert.h>
#include <stdio.h> // warning/error messages

#include "vectormath.hpp"


/////////////
//Common FRaC

Array<SampleScorePair> FRaC::scoreTestSet(Array<Sample> in){
  
  Array<SampleScorePair> out(in.length);
  
  for(unsigned i = 0; i < out.length; i++){
    out[i] = SampleScorePair(in[i], calculateAnomalyScore(in[i]));
  }
  
  return out;
}

Array<SampleScorePair> FRaC::scoreTestSet(Array<Sample> in, std::ostream& log){
  
  Array<SampleScorePair> out(in.length);
  
  for(unsigned i = 0; i < out.length; i++){
    out[i] = SampleScorePair(in[i], calculateAnomalyScore(in[i], log));
    log << std::endl;
  }
  
  return out;
}

Array<SampleScorePair> FRaC::rankTestSet(Array<Sample> in){
  //Array<SampleScorePair> out = in.map<SampleScorePair>([](Sample s){return SampleScorePair(s, calculateAnomalyScore(s));}); //Needs a closure
  
  Array<SampleScorePair> out = scoreTestSet(in);
  
  out.sort();
  return out;
}

//////////////////
//TRADITIONAL FRAC

fracfloat_t TraditionalFRaC::calculateNS(Sample trueSample){
 
  fracfloat_t ns = 0;
  
  unsigned knownCount = 0;

  for(unsigned i = 0; i < sampleStructure.contvarCount; i++){
    //std::cout << trueSample.contvars[i] << std::endl;
    if(!trueSample.contvarUnknown(i))
    {
      knownCount++;
      for(unsigned j = 0; j < errorModels.continuousModelCount; j++){
        ns += errorModels.getContinuousModel(i, j)->surprisal(trueSample.contvars[i], predictor.getRegressor(i, j)->predict(trueSample));
      }
    }
    else{
      //std::cout << "Found unknown sample: " << i << std::endl;
      ns += errorModels.continuousModelCount * (*errorModels.continuousFeatureEntropy(i));
    }
  }
  
  for(unsigned i = 0; i < sampleStructure.catvarCount; i++){
    if(!trueSample.catvarUnknown(i))
    {
      knownCount++;
      for(unsigned j = 0; j < errorModels.categoricalModelCount; j++){
        ns += errorModels.getCategoricalModel(i, j)->surprisal(trueSample.catvars[i], predictor.getClassifier(i, j)->predict(trueSample));
      }
    }
    else{
      ns += errorModels.categoricalModelCount * (*errorModels.categoricalFeatureEntropy(i));
    }
  }
  
  for(unsigned i = 0; i < sampleStructure.binvarCount; i++){
    if(!trueSample.binvarUnknown(i))
    {
      knownCount++;
      for(unsigned j = 0; j < errorModels.binaryModelCount; j++){
        ns += errorModels.getBinaryModel(i, j)->surprisal(trueSample.binvars[i], predictor.getBinaryClassifier(i, j)->predict(trueSample));
      }
    }
    else{
      ns += errorModels.binaryModelCount * (*errorModels.binaryFeatureEntropy(i));
    }
  }
  
  //Normalize surprisal values
  ns -= errorModels.continuousModelCount * errorModels.sumContinuousEntropyValues() + 
        errorModels.categoricalModelCount * errorModels.sumCategoricalEntropyValues() + 
        errorModels.binaryModelCount * errorModels.sumBinaryEntropyValues();
  
  //Missing feature correction
  
  if(applyUnknownCountCorrection){
    ns = ns * sampleStructure.featureCount() / (fracfloat_t) knownCount;
  }
  
  return ns;
}

//////////////////
//DIVISION FRAC

fracfloat_t DivisionFRaC::calculateNS(Sample trueSample){
 
  fracfloat_t ns = 0;
  
  unsigned knownCount = 0;

  for(unsigned i = 0; i < sampleStructure.contvarCount; i++){
    if(!trueSample.contvarUnknown(i))
    {
      knownCount++;
      fracfloat_t termSurprisal = 0;
      for(unsigned j = 0; j < errorModels.continuousModelCount; j++){
        termSurprisal += errorModels.getContinuousModel(i, j)->surprisal(trueSample.contvars[i], predictor.getRegressor(i, j)->predict(trueSample));
      }
      if(applyPresubtraction) termSurprisal -= errorModels.continuousModelCount * *errorModels.continuousFeatureEntropy(i);
      termSurprisal /= *errorModels.continuousFeatureEntropy(i); //Here the division of DivisionFRaC occurs.
      ns += termSurprisal;
    }
  }
  
  for(unsigned i = 0; i < sampleStructure.catvarCount; i++){
    if(!trueSample.catvarUnknown(i))
    {
      knownCount++;
      fracfloat_t termSurprisal = 0;
      for(unsigned j = 0; j < errorModels.categoricalModelCount; j++){
        termSurprisal += errorModels.getCategoricalModel(i, j)->surprisal(trueSample.catvars[i], predictor.getClassifier(i, j)->predict(trueSample));
      }
      if(applyPresubtraction) termSurprisal -= errorModels.categoricalModelCount * *errorModels.categoricalFeatureEntropy(i);
      termSurprisal /= *errorModels.categoricalFeatureEntropy(i); //Here the division of DivisionFRaC occurs.
      ns += termSurprisal;
    }
  }
  
  for(unsigned i = 0; i < sampleStructure.binvarCount; i++){
    if(!trueSample.binvarUnknown(i))
    {
      knownCount++;
      fracfloat_t termSurprisal = 0;
      for(unsigned j = 0; j < errorModels.binaryModelCount; j++){
        termSurprisal += errorModels.getBinaryModel(i, j)->surprisal(trueSample.binvars[i], predictor.getBinaryClassifier(i, j)->predict(trueSample));
      }
      if(applyPresubtraction) termSurprisal -= errorModels.binaryModelCount * *errorModels.binaryFeatureEntropy(i);
      termSurprisal /= (*errorModels.binaryFeatureEntropy(i));
      ns += termSurprisal;
    }
  }
  
  //Missing feature correction
  
  if(applyUnknownCountCorrection){
    ns = ns * sampleStructure.featureCount() / (fracfloat_t) knownCount;
  }
  
  return ns;
}

//////////////
//Generic FRaC

fracfloat_t GenericFRaC::calculateAnomalyScore(Sample trueSample){
  
  fracfloat_t ns = 0;
  
  unsigned knownCount = 0;

  for(unsigned i = 0; i < sampleStructure.contvarCount; i++){
    if(!trueSample.contvarUnknown(i))
    {
      knownCount++;
      for(unsigned j = 0; j < errorModels.continuousModelCount; j++){
        fracfloat_t surprisal = errorModels.getContinuousModel(i, j)->surprisal(trueSample.contvars[i], predictor.getRegressor(i, j)->predict(trueSample));
        fracfloat_t entropy = *errorModels.continuousFeatureEntropy(i);
        fracfloat_t termSurprisal = calculateContinuousTermScore(surprisal, entropy, i, j);
        ns += termSurprisal;
      }
    }
  }
  
  for(unsigned i = 0; i < sampleStructure.catvarCount; i++){
    if(!trueSample.catvarUnknown(i))
    {
      knownCount++;
      for(unsigned j = 0; j < errorModels.categoricalModelCount; j++){
        fracfloat_t surprisal = errorModels.getCategoricalModel(i, j)->surprisal(trueSample.catvars[i], predictor.getClassifier(i, j)->predict(trueSample));
        fracfloat_t entropy = *errorModels.categoricalFeatureEntropy(i);
        fracfloat_t termSurprisal = calculateCategoricalTermScore(surprisal, entropy, i, j);
        ns += termSurprisal;
      }
    }
  }
  
  for(unsigned i = 0; i < sampleStructure.binvarCount; i++){
    if(!trueSample.binvarUnknown(i))
    {
      knownCount++;
      for(unsigned j = 0; j < errorModels.categoricalModelCount; j++){
        fracfloat_t surprisal = errorModels.getBinaryModel(i, j)->surprisal(trueSample.binvars[i], predictor.getBinaryClassifier(i, j)->predict(trueSample));
        fracfloat_t entropy = *errorModels.binaryFeatureEntropy(i);
        fracfloat_t termSurprisal = calculateBinaryTermScore(surprisal, entropy, i, j);
        ns += termSurprisal;
      }
    }
  }
  
  //Missing feature correction
  
  if(applyUnknownCountCorrection){
    ns = ns * sampleStructure.featureCount() / (fracfloat_t) knownCount;
  }
  
  return ns;
}

//Avert your eyes.
//#define PRINT_CHECK_FIRST(val,o) (first?(first=false,o):(o<<"\t"))<<val
//You were warned.

#define PRINT_ZERO_ENTRIES(o,count) for(unsigned i = 0; i < count; i++){o << "0\t";}

fracfloat_t GenericFRaC::calculateAnomalyScore(Sample trueSample, std::ostream& logStream){
  
  fracfloat_t ns = 0;
  unsigned knownCount = 0;

  for(unsigned i = 0; i < sampleStructure.contvarCount; i++){
    if(!trueSample.contvarUnknown(i))
    {
      knownCount++;
      for(unsigned j = 0; j < errorModels.continuousModelCount; j++){
        fracfloat_t surprisal = errorModels.getContinuousModel(i, j)->surprisal(trueSample.contvars[i], predictor.getRegressor(i, j)->predict(trueSample));
        fracfloat_t entropy = *errorModels.continuousFeatureEntropy(i);
        fracfloat_t termSurprisal = calculateContinuousTermScore(surprisal, entropy, i, j);
        ns += termSurprisal;
        
        logStream << termSurprisal << "\t";
      }
    }
    else{
      PRINT_ZERO_ENTRIES(logStream, errorModels.continuousModelCount);
    }
  }
  
  for(unsigned i = 0; i < sampleStructure.catvarCount; i++){
    if(!trueSample.catvarUnknown(i))
    {
      knownCount++;
      for(unsigned j = 0; j < errorModels.categoricalModelCount; j++){
        fracfloat_t surprisal = errorModels.getCategoricalModel(i, j)->surprisal(trueSample.catvars[i], predictor.getClassifier(i, j)->predict(trueSample));
        fracfloat_t entropy = *errorModels.categoricalFeatureEntropy(i);
        fracfloat_t termSurprisal = calculateCategoricalTermScore(surprisal, entropy, i, j);
        ns += termSurprisal;
        
        logStream << termSurprisal << "\t";
      }
    }
    else{
      PRINT_ZERO_ENTRIES(logStream, errorModels.categoricalModelCount);
    }
  }
  
  for(unsigned i = 0; i < sampleStructure.binvarCount; i++){
    if(!trueSample.binvarUnknown(i))
    {
      knownCount++;
      for(unsigned j = 0; j < errorModels.binaryModelCount; j++){
        fracfloat_t surprisal = errorModels.getBinaryModel(i, j)->surprisal(trueSample.binvars[i], predictor.getBinaryClassifier(i, j)->predict(trueSample));
        fracfloat_t entropy = *errorModels.binaryFeatureEntropy(i);
        fracfloat_t termSurprisal = calculateBinaryTermScore(surprisal, entropy, i, j);
        ns += termSurprisal;
        
        logStream << termSurprisal << "\t";
      }
    }
    else{
      PRINT_ZERO_ENTRIES(logStream, errorModels.binaryModelCount);
    }
  }
  
  //Missing feature correction
  
  if(applyUnknownCountCorrection){
    ns = ns * sampleStructure.featureCount() / (fracfloat_t) knownCount;
  }
  
  //Cap the line with one last entry.
  //This is not redundant because this is the only term with the unknown correction
  logStream << ns;
  
  return ns;
}

//Generic implementation of traditional FRaC
fracfloat_t TraditionalFRaC_G::calculateTermScore(fracfloat_t surprisal, fracfloat_t entropy){
  return surprisal - entropy;
}
//Generic implementation of division FRaC

fracfloat_t DivisionFRaC_G::calculateTermScore(fracfloat_t surprisal, fracfloat_t entropy){
  return (surprisal - entropy) / entropy;
}



////////////////////
//Probabilistic FRaC

fracfloat_t ProbabilisticFRaC::calculateNS(Sample trueSample){
  
  //Calculate expected normalized surprisal
  
  fracfloat_t ns = 0;
  
  unsigned knownCount = 0;
  
  for(unsigned i = 0; i < sampleStructure.contvarCount; i++){
    if(!trueSample.contvarUnknown(i))
    {
      //TODO INTEGRAL
      fracfloat_t predictedVal = predictor.regressors[i]->predict(trueSample);

      //TODO
      //ns += errorModels.regressorErrorModels[i]->surprisal(trueSample.contvars[i], predictedVal);
      knownCount++;
    }
  }
  
  fracfloat_t catvarDistMem[max<unsigned>(sampleStructure.catvarSizes, sampleStructure.catvarCount)];
  
  for(unsigned i = 0; i < sampleStructure.catvarCount; i++){
    if(!trueSample.catvarUnknown(i))
    {
      predictor.classifiers[i]->predictDistribution(trueSample, catvarDistMem);
      
      fracfloat_t thisTermNS = 0;
      
      //Expectation calculation
      for(unsigned j = 0; j < sampleStructure.catvarSizes[i]; j++){
      //TODO
      //  thisTermNS += catvarDistMem[j] * errorModels.classifierErrorModels[i]->surprisal(trueSample.catvars[i], j);
      }
      
      ns += thisTermNS;
      
      knownCount++;
    }
  }
  
  for(unsigned i = 0; i < sampleStructure.binvarCount; i++){
    if(!trueSample.binvarUnknown(i))
    {
      fracfloat_t dist = predictor.binaryClassifiers[i]->predictDistribution(trueSample);
      //TODO
      //ns  +=  dist  * errorModels.binaryClassifierErrorModels[i]->surprisal(trueSample.binvars[i], 0) +
      //   (1 - dist) * errorModels.binaryClassifierErrorModels[i]->surprisal(trueSample.binvars[i], 0);
      //Optimization: The above might be simplifiable.
      
      knownCount++;
    }
  }
  
  //Missing feature correction
  
  if(applyUnknownCountCorrection){
    ns = ns * sampleStructure.featureCount() / (double)knownCount;
  }
  
  return ns;
}