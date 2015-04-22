#ifndef FRAC_H
#define FRAC_H

#include "errormodelcollection.hpp"
#include "predictor.hpp"

#include <stdio.h>

#include <math.h> // log, logl
#include <assert.h>

struct SampleScorePair{
  fracfloat_t anomalyScore;
  Sample sample;
  
  SampleScorePair(){}
  SampleScorePair(Sample s, fracfloat_t anomalyScore) : anomalyScore(anomalyScore), sample(s) { }
  
  bool operator < (const SampleScorePair& other) const
  {
    return (anomalyScore < other.anomalyScore);
  }
};

class FRaC {
  public:
  
  virtual fracfloat_t calculateAnomalyScore(Sample trueSample) = 0;

  //Default implementation
  virtual fracfloat_t calculateAnomalyScore(Sample trueSample, std::ostream& log){
    return calculateAnomalyScore(trueSample);
  }
  
  Array<SampleScorePair> scoreTestSet(Array<Sample> in);
  Array<SampleScorePair> rankTestSet(Array<Sample> in);
  
  Array<SampleScorePair> scoreTestSet(Array<Sample> in, std::ostream& log);
  
  virtual ~FRaC(){}; //Deletion of a polymorphic object is undefined without virtual deconstructor.  Even though we don't use the deconstructor.
  virtual void freeMemory(){}

};

class TraditionalFRaC : public FRaC {
private: //Not worth it.
public:
    SampleStructure sampleStructure;
    Predictor predictor;
    ErrorModelCollection errorModels;
    
    bool applyUnknownCountCorrection;
    
public:
    //Constructors:
    TraditionalFRaC(SampleStructure st, Predictor p, ErrorModelCollection e) : sampleStructure(st), predictor(p), errorModels(e), applyUnknownCountCorrection(false) {
      assert(e.continuousModelCount == p.regressorCount);
      assert(e.categoricalModelCount == p.classifierCount);
      assert(e.binaryModelCount == p.binaryClassifierCount);
    }
    
    TraditionalFRaC(SampleStructure st, Predictor p, ErrorModelCollection e, bool unknownCorrection) : TraditionalFRaC(st, p, e) {
      applyUnknownCountCorrection = unknownCorrection;
    }

    
    //Functionality:
    fracfloat_t calculateNS(Sample trueSample);
    
    fracfloat_t calculateAnomalyScore(Sample trueSample){
      return calculateNS(trueSample);
    }
    
    ~TraditionalFRaC(){}
    
    void freeMemory(){
      predictor.freeMemory();
      errorModels.freeMemory();
    }
};

class DivisionFRaC : public FRaC {
private: //Not worth it.
public:
    SampleStructure sampleStructure;
    Predictor predictor;
    ErrorModelCollection errorModels;
    
    bool applyUnknownCountCorrection;
    bool applyPresubtraction;
    
public:
    //Constructors:
    DivisionFRaC(SampleStructure st, Predictor p, ErrorModelCollection e) : sampleStructure(st), predictor(p), errorModels(e), applyUnknownCountCorrection(false), applyPresubtraction(false) {
      assert(e.continuousModelCount == p.regressorCount);
      assert(e.categoricalModelCount == p.classifierCount);
      assert(e.binaryModelCount == p.binaryClassifierCount);
    }
    
    DivisionFRaC(SampleStructure st, Predictor p, ErrorModelCollection e, bool unknownCorrection, bool applyPresubtraction) : DivisionFRaC(st, p, e) {
      applyUnknownCountCorrection = unknownCorrection;
      applyPresubtraction = applyPresubtraction;
    }

    
    //Functionality:
    fracfloat_t calculateNS(Sample trueSample);
    
    fracfloat_t calculateAnomalyScore(Sample trueSample){
      return calculateNS(trueSample);
    }
    
    ~DivisionFRaC(){}
    
    void freeMemory(){
      predictor.freeMemory();
      errorModels.freeMemory();
    }
};

class ProbabilisticFRaC : public FRaC {
  SampleStructure sampleStructure;
  ProbabilisticPredictor predictor;
  ErrorModelCollection errorModels;
  
  bool applyUnknownCountCorrection;

  ProbabilisticFRaC(SampleStructure st, ProbabilisticPredictor p, ErrorModelCollection e) : sampleStructure(st), predictor(p), errorModels(e), applyUnknownCountCorrection(false) { }
    
  //Functionality:
  fracfloat_t calculateNS(Sample trueSample);
    
  fracfloat_t calculateAnomalyScore(Sample trueSample){
    return calculateNS(trueSample);
  }
};

/*
class ApFRaC : public FRaC {
  //TODO: Like TraditionalFrac, but using likelihood calculations instead.
};
 */

//GENERIC FRAC STUFF:

class GenericFRaC : public FRaC {
  public:
  SampleStructure sampleStructure;
  Predictor predictor;
  ErrorModelCollection errorModels;

  bool applyUnknownCountCorrection;

  GenericFRaC(SampleStructure st, Predictor p, ErrorModelCollection e, bool applyUnknownCountCorrection) : sampleStructure(st), predictor(p), errorModels(e), applyUnknownCountCorrection(applyUnknownCountCorrection){
    assert(e.continuousModelCount == p.regressorCount);
    assert(e.categoricalModelCount == p.classifierCount);
    assert(e.binaryModelCount == p.binaryClassifierCount);
  }
    
  fracfloat_t calculateAnomalyScore(Sample trueSample);
  
  fracfloat_t calculateAnomalyScore(Sample trueSample, std::ostream& log);
  
  //Generic term calculation
  virtual fracfloat_t calculateTermScore(fracfloat_t surprisal, fracfloat_t entropy) = 0;
  
  //Specific term calculation, probably not used.
  virtual fracfloat_t calculateContinuousTermScore(fracfloat_t surprisal, fracfloat_t entropy, unsigned featureIndex, unsigned predictorIndex){
    return calculateTermScore(surprisal, entropy);
  }
  virtual fracfloat_t calculateCategoricalTermScore(fracfloat_t surprisal, fracfloat_t entropy, unsigned featureIndex, unsigned predictorIndex){
    return calculateTermScore(surprisal, entropy);
  }
  virtual fracfloat_t calculateBinaryTermScore(fracfloat_t surprisal, fracfloat_t entropy, unsigned featureIndex, unsigned predictorIndex){
    return calculateTermScore(surprisal, entropy);
  }
  
  void freeMemory(){
    predictor.freeMemory();
    errorModels.freeMemory();
  }
};

class TraditionalFRaC_G : public GenericFRaC {
  public:
  
  TraditionalFRaC_G(SampleStructure st, Predictor p, ErrorModelCollection e, bool missingValueCorrection) : GenericFRaC(st, p, e, missingValueCorrection) { }
  
  fracfloat_t calculateTermScore(fracfloat_t surprisal, fracfloat_t entropy);
};

class DivisionFRaC_G : public GenericFRaC {
  public:
    
  DivisionFRaC_G(SampleStructure st, Predictor p, ErrorModelCollection e, bool applyUnknownCountCorrection) : GenericFRaC(st, p, e, applyUnknownCountCorrection) { }
    
  fracfloat_t calculateTermScore(fracfloat_t surprisal, fracfloat_t entropy);
};

//Partial filtering FRaC.  Note that this is a research implementation, and it's only marginally faster than ordinary FRaC (it still foolishly trains models for partially filtered features, it just doesn't use them).
class pfFRaC_G : public GenericFRaC {
  public:
  BitArray partialFilter;

  pfFRaC_G(SampleStructure st, Predictor p, ErrorModelCollection e, BitArray partialFilter) : GenericFRaC(st, p, e, false), partialFilter(partialFilter) { }
  
  //Generic term calculation
  fracfloat_t calculateTermScore(fracfloat_t surprisal, fracfloat_t entropy){
    return surprisal - entropy;
  }
  
  //Check the filter in term calculation
  fracfloat_t calculateContinuousTermScore(fracfloat_t surprisal, fracfloat_t entropy, unsigned featureIndex, unsigned predictorIndex){
    if(partialFilter[sampleStructure.contvarIndex(featureIndex)]) return calculateTermScore(surprisal, entropy);
    else return 0;
  }
  fracfloat_t calculateCategoricalTermScore(fracfloat_t surprisal, fracfloat_t entropy, unsigned featureIndex, unsigned predictorIndex){
    if(partialFilter[sampleStructure.catvarIndex(featureIndex)]) return calculateTermScore(surprisal, entropy);
    else return 0;
  }
  fracfloat_t calculateBinaryTermScore(fracfloat_t surprisal, fracfloat_t entropy, unsigned featureIndex, unsigned predictorIndex){
    if(partialFilter[sampleStructure.binvarIndex(featureIndex)]) return calculateTermScore(surprisal, entropy);
    else return 0;
  }
};

//For sFRaC
class WeightedFRaC : public GenericFRaC {
  fracfloat_t* weights;
  fracfloat_t calculateTermScore(fracfloat_t surprisal, fracfloat_t entropy, unsigned index);
};

#endif
