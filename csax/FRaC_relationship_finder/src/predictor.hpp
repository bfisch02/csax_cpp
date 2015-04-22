#ifndef PREDICTOR_H
#define PREDICTOR_H

#include "array.hpp"
#include "sample.hpp"
#include "mltypes.hpp"

//Typedef some factory functions.

//Creator functions create an ML object (regressor or classifier, and take the sample type they operate on, an array of data to build from, and an unsigned, representing the index of the feature to construct on).
typedef Regressor* (*RegressorCreator) (SampleStructure*, Array<Sample>, unsigned);
typedef Classifier* (*ClassifierCreator) (SampleStructure*, Array<Sample>, unsigned);
typedef BinaryClassifier* (*BinaryClassifierCreator) (SampleStructure*, Array<Sample>, unsigned);

struct MlCreators{
  RegressorCreator* rc;
  ClassifierCreator* cc;
  BinaryClassifierCreator* bcc;
  
  unsigned regressorCount;
  unsigned classifierCount;
  unsigned binaryClassifierCount;
  
  void freeMemory(){
    delete [] rc;
    delete [] cc;
    delete [] bcc;
  }
};

//A predictor object, holds regressors and classifiers for a sample type.
class Predictor{
  private://Not worth it.
  public:
  SampleStructure ss;

  //These are polymorphic arrays, so we unfortunately need the **.
  Regressor** regressors;
  Classifier** classifiers;
  BinaryClassifier** binaryClassifiers;
  
  unsigned regressorCount;
  unsigned classifierCount;
  unsigned binaryClassifierCount;
  
  public:
      
  Predictor(){}

  Predictor(SampleStructure structure, Regressor** regressors, Classifier** classifiers, BinaryClassifier** binaryClassifiers, unsigned continuousModelCount, unsigned categoricalModelCount, unsigned binaryModelCount) : ss(structure), regressors(regressors), classifiers(classifiers), binaryClassifiers(binaryClassifiers), regressorCount(continuousModelCount), classifierCount(categoricalModelCount), binaryClassifierCount(binaryModelCount) { }

  Predictor(SampleStructure structure, Array<Sample> samples, MlCreators);
  
  //Accessors for regressors, classifiers, and binary classifiers
  
  Sample predict(Sample inSample, unsigned regressorIndex, unsigned classifierIndex, unsigned binaryClassifierIndex);
  
  Regressor** getRegressorPtr(unsigned featureIndex, unsigned modelIndex){
    assert(featureIndex < ss.contvarCount);
    assert(modelIndex < regressorCount);
    return &regressors[featureIndex * regressorCount + modelIndex];
  }
  
  Regressor* getRegressor(unsigned featureIndex, unsigned modelIndex){
    return *getRegressorPtr(featureIndex, modelIndex);
  }

  Classifier** getClassifierPtr(unsigned featureIndex, unsigned modelIndex){
    assert(featureIndex < ss.catvarCount);
    assert(modelIndex < classifierCount);
    return &classifiers[featureIndex * classifierCount + modelIndex];
  }
  
  Classifier* getClassifier(unsigned featureIndex, unsigned modelIndex){
      return *getClassifierPtr(featureIndex, modelIndex);
  }
  
  BinaryClassifier** getBinaryClassifierPtr(unsigned featureIndex, unsigned modelIndex){
    assert(featureIndex < ss.binvarCount);
    assert(modelIndex < binaryClassifierCount);
    return &binaryClassifiers[featureIndex * binaryClassifierCount + modelIndex];
  }
  BinaryClassifier* getBinaryClassifier(unsigned featureIndex, unsigned modelIndex){
      return *getBinaryClassifierPtr(featureIndex, modelIndex);
  }
  
  void freeMemory();
};

//A probabilistic predictor object, holds regressors and classifiers for a sample type.;
class ProbabilisticPredictor{;
  public:
  SampleStructure structure;

  //These are polymorphic arrays, so we unfortunately need the **.
  ProbabilisticRegressor** regressors;
  ProbabilisticClassifier** classifiers;
  ProbabilisticBinaryClassifier** binaryClassifiers;
  
  public:

  ProbabilisticPredictor(SampleStructure structure, ProbabilisticRegressor** regressors, ProbabilisticClassifier** classifiers, ProbabilisticBinaryClassifier** binaryClassifiers) : structure(structure), regressors(regressors), classifiers(classifiers), binaryClassifiers(binaryClassifiers) { }

  ProbabilisticPredictor(SampleStructure structure, Array<Sample> samples, MlCreators);

  //Architecture: Could have functions that use the innards.
  /*
   Gaussian predictContinuous(Sample s, unsigned index);
   */
};

#endif
