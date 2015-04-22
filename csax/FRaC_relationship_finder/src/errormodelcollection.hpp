#ifndef ERRORMODELCOLLECTION_H
#define	ERRORMODELCOLLECTION_H

#include "errormodel.hpp"
#include "predictor.hpp"

//These are analogous to the MlCreators of a predictor

//Consider: Less indirection on SampleStructures?

//ErrorModel type, name, SampleType, true values, predicted values, values count.
typedef ContinuousErrorModel* (*ContinuousErrorModelCreator) (SampleStructure*, unsigned, contvar_t*, contvar_t*, unsigned);
typedef CategoricalErrorModel* (*CategoricalErrorModelCreator) (SampleStructure*, unsigned, catvar_t*, catvar_t*, unsigned);
typedef BinaryErrorModel* (*BinaryErrorModelCreator) (SampleStructure*, unsigned, binvar_t*, binvar_t*, unsigned); //Consider: use bitvectors?

struct ErrorModelCreators{
  ContinuousErrorModelCreator rc;
  CategoricalErrorModelCreator cc;
  BinaryErrorModelCreator bcc;
  
  //Architecture: for heterogenous ErrorModelCreators, we would need this:
  /*
  
  ContinuousErrorModelCreator* rc;
  CategoricalErrorModelCreator* cc;
  BinaryErrorModelCreator* bcc;
  
  unsigned regressorCount;
  unsigned classifierCount;
  unsigned binaryClassifierCount;
  
  void freeMemory(){
    delete [] rc;
    delete [] cc;
    delete [] bcc;
  }
  
   */
};


//The error model analog of a Predictor.  Must be interpreted in light of a SampleType.
class ErrorModelCollection {
  public:
  
  //Data
  
  ContinuousErrorModel** continuousErrorModels;
  CategoricalErrorModel** categoricalErrorModels;
  BinaryErrorModel** binaryErrorModels;
  
  SampleStructure ss;
  
  unsigned continuousModelCount;
  unsigned categoricalModelCount;
  unsigned binaryModelCount;
  
  fracfloat_t* entropyValues;
  
  public:
  //Constructors

  ErrorModelCollection(){};
  
  ErrorModelCollection(ContinuousErrorModel** regressorErrorModels, CategoricalErrorModel** classifierErrorModels, BinaryErrorModel** binaryClassifierErrorModels, SampleStructure ss, unsigned continuousModelCount, unsigned categoricalModelCount, unsigned binaryModelCount, fracfloat_t* entropyValues) :
  continuousErrorModels(regressorErrorModels), categoricalErrorModels(classifierErrorModels), binaryErrorModels(binaryClassifierErrorModels), ss(ss), continuousModelCount(continuousModelCount), categoricalModelCount(categoricalModelCount), binaryModelCount(binaryModelCount), entropyValues(entropyValues) { }

  ErrorModelCollection(SampleStructure sampleStructure, ErrorModelCreators emc, Array<Sample> trainingData, Array<Sample> validationData, Predictor p);
  
  //Cross Validation constructor.
  ErrorModelCollection(MlCreators mlc, ErrorModelCreators emc, StructuredSampleCollection samples, unsigned folds, unsigned shuffles);
  
  // Error models
  
  ContinuousErrorModel** getContinuousModelPtr(unsigned featureIndex, unsigned modelIndex){
    assert(featureIndex < ss.contvarCount);
    assert(modelIndex < continuousModelCount);
    return &continuousErrorModels[featureIndex * continuousModelCount + modelIndex];
  }
  ContinuousErrorModel* getContinuousModel(unsigned featureIndex, unsigned modelIndex){
    return *getContinuousModelPtr(featureIndex, modelIndex);
  }

  CategoricalErrorModel** getCategoricalModelPtr(unsigned featureIndex, unsigned modelIndex){
    assert(featureIndex < ss.catvarCount);
    assert(modelIndex < categoricalModelCount);
    return &categoricalErrorModels[featureIndex * categoricalModelCount + modelIndex];
  }
  CategoricalErrorModel* getCategoricalModel(unsigned featureIndex, unsigned modelIndex){
    return *getCategoricalModelPtr(featureIndex, modelIndex);
  }
  
  BinaryErrorModel** getBinaryModelPtr(unsigned featureIndex, unsigned modelIndex){
    assert(featureIndex < ss.binvarCount);
    assert(modelIndex < binaryModelCount);
    return &binaryErrorModels[featureIndex * binaryModelCount + modelIndex];
  }
  BinaryErrorModel* getBinaryModel(unsigned featureIndex, unsigned modelIndex){
    return *getBinaryModelPtr(featureIndex, modelIndex);
  }
  
  //Entropy
  
  fracfloat_t* continuousFeatureEntropy(unsigned featureIndex){
    assert(featureIndex < ss.contvarCount);
    return entropyValues + featureIndex;
  }
  
  fracfloat_t* categoricalFeatureEntropy(unsigned featureIndex){
    assert(featureIndex < ss.catvarCount);
    return entropyValues + ss.contvarCount + featureIndex;
  }
  
  fracfloat_t* binaryFeatureEntropy(unsigned featureIndex){
    assert(featureIndex < ss.binvarCount);
    return entropyValues + ss.contvarCount + ss.catvarCount + featureIndex;
  }
  
  fracfloat_t sumEntropyValues(){
    return sumTerms<fracfloat_t>(entropyValues, ss.featureCount());
  }

  fracfloat_t sumContinuousEntropyValues(){
    return sumTerms<fracfloat_t>(entropyValues + 0, ss.contvarCount);
  }
  fracfloat_t sumCategoricalEntropyValues(){
    return sumTerms<fracfloat_t>(entropyValues + ss.contvarCount, ss.catvarCount);
  }
  fracfloat_t sumBinaryEntropyValues(){
    return sumTerms<fracfloat_t>(entropyValues + ss.contvarCount + ss.catvarCount, ss.binvarCount);
  }
  
  void freeMemory();
  
  private:
  void buildErrorModels(SampleStructure sampleStructure, ErrorModelCreators emc, Array<Sample> truth, Array<Sample> predictions);
  void buildErrorModels(SampleStructure sampleStructure, ErrorModelCreators emc, contvar_t* trueContvars, contvar_t* predContvars, catvar_t* trueCatvars, catvar_t* predCatvars, binvar_t* trueBinvars, binvar_t* predBinvars, unsigned shuffleCount, unsigned sampleCount);

  void calculateEntropies(Array<Sample> samples);
};


#endif