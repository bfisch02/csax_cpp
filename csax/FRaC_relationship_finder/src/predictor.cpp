#include "predictor.hpp"

#include <algorithm>

Predictor::Predictor(SampleStructure structure, Array<Sample> samples, MlCreators m) : ss(structure), regressorCount(m.regressorCount), classifierCount(m.classifierCount), binaryClassifierCount(m.binaryClassifierCount) {
  
  regressors = new Regressor*[ss.contvarCount * regressorCount];
  classifiers = new Classifier*[ss.catvarCount * classifierCount];
  binaryClassifiers = new BinaryClassifier*[ss.binvarCount * binaryClassifierCount];
  
  #define THREADED
  #ifdef THREADED

  //Optimization: This is a complete waste, needs to be replaced by a counting iterator of some sort.
  #define MAX2(a,b) (((a)>(b))?(a):(b))
  #define MAX3(a,b,c) MAX2((a),MAX2((b),(c)))
  unsigned maxCt = MAX3(ss.contvarCount, ss.catvarCount, ss.binvarCount);
  unsigned countIndices [maxCt];
  for(unsigned i = 0; i < maxCt; i++){
    countIndices[i] = i;
  }
    
  std::for_each(countIndices, countIndices + ss.contvarCount, [&](unsigned i){
    for(unsigned j = 0; j < regressorCount; j++){
      *getRegressorPtr(i, j) = (*m.rc[j]) (&ss, samples, i);
    }
  }, __gnu_parallel::parallel_balanced);
  
  std::for_each(countIndices, countIndices + ss.catvarCount, [&](unsigned i){ 
    for(unsigned j = 0; j < classifierCount; j++){
      *getClassifierPtr(i, j) = (*m.cc[j]) (&ss, samples, i);
    }
  }, __gnu_parallel::parallel_balanced);
  
  
  std::for_each(countIndices, countIndices + ss.binvarCount, [&](unsigned i){ 
    for(unsigned j = 0; j < binaryClassifierCount; j++){
      *getBinaryClassifierPtr(i, j) = (*m.bcc[j]) (&ss, samples, i);
    }
  }, __gnu_parallel::parallel_balanced);
        
    
  #else

  for(unsigned i = 0; i < ss.contvarCount; i++){
    for(unsigned j = 0; j < regressorCount; j++){
      *getRegressorPtr(i, j) = (*m.rc[j]) (&ss, samples, i);
    }
  }
  
  for(unsigned i = 0; i < ss.catvarCount; i++){
    for(unsigned j = 0; j < classifierCount; j++){
      *getClassifierPtr(i, j) = (*m.cc[j]) (&ss, samples, i);
    }
  }
  
  for(unsigned i = 0; i < ss.binvarCount; i++){
    for(unsigned j = 0; j < binaryClassifierCount; j++){
      *getBinaryClassifierPtr(i, j) = (*m.bcc[j]) (&ss, samples, i);
    }
  }
    
  #endif

}

Sample Predictor::predict(Sample inSample, unsigned regressorIndex, unsigned classifierIndex, unsigned binaryClassifierIndex) {
  Sample outSample = Sample(ss);
  
  //This could be parallelized as well.
  for(unsigned i = 0; i < ss.contvarCount; i++){
    if(!inSample.contvarUnknown(i)) //Should be optimized out if unknown values are off.
      outSample.contvars[i] = getRegressor(i, regressorIndex)->predict(inSample);
    //Otherwise leave it uninitialized: it won't be used.
  }
  
  for(unsigned i = 0; i < ss.catvarCount; i++){
    if(!inSample.catvarUnknown(i))
      outSample.catvars[i] = getClassifier(i, classifierIndex)->predict(inSample);
  }
  
  for(unsigned i = 0; i < ss.binvarCount; i++){
    if(!inSample.binvarUnknown(i))
      outSample.binvars[i] = getBinaryClassifier(i, binaryClassifierIndex)->predict(inSample);
  }
  
  return outSample;
}

void Predictor::freeMemory(){
    
  
  Array<Regressor*>(regressors, ss.contvarCount * regressorCount).forEach([](Regressor*& m){m->freeMemory(); delete m;});
  delete [] regressors;

  Array<Classifier*>(classifiers, ss.catvarCount * classifierCount).forEach([](Classifier*& m){m->freeMemory(); delete m;});
  delete [] classifiers;

  Array<BinaryClassifier*>(binaryClassifiers, ss.binvarCount * binaryClassifierCount).forEach([](BinaryClassifier*& m){m->freeMemory(); delete m;});
  delete [] binaryClassifiers;
}