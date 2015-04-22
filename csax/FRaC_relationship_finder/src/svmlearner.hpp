#ifndef SVMLEARNER_H
#define	SVMLEARNER_H

#include "array.hpp"
#include "mltypes.hpp"
#include "svm.h"


class SvmRegressor : public Regressor{
  private:
  public:
  SampleStructure st;
  unsigned featureIndex;
  
  svm_parameter* params;
  svm_model model;
  
  svm_node* blob;
  

  SvmRegressor(){}//For array construction.
  SvmRegressor(SampleStructure sampleType, unsigned featureIndex, svm_parameter* params) : st(sampleType), featureIndex(featureIndex), params(params){ }
  
  fracfloat_t predict(Sample);
    
  void train(Array<Sample> training);

  void writeToStream(std::ostream& o) const{
    o << "{SVM regressor}"; //Would be good to have some more detail.
    //params?
  }
  
  void freeMemory(){
      svm_free_model_content(&model);
      delete [] blob;
  }
  
  private:    
  fracfloat_t contvarValResolveUnknowns(Sample s, unsigned index);
  void loadNode(svm_node* nodeInWhichToLoad, Sample s);
  unsigned columnCount();
};

#endif	/* SVMLEARNER_H */

