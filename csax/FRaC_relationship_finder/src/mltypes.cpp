#include "mltypes.hpp"

//Global for deterministic cycling. (Not thread deterministic, but still random enough).
unsigned random_cycle = 0;

//Probabilistic Learners:

catvar_t ProbabilisticClassifier::predict(Sample s){
  unsigned len = predictDistributionClassCount();
  fracfloat_t distributionClasses[len];
  predictDistribution(s, distributionClasses);

  assert(epsilonCompare<fracfloat_t>(sumTerms<fracfloat_t>(distributionClasses, len), (fracfloat_t)1));
  int mui = maxUniqueIndex<fracfloat_t>(distributionClasses, len);

  if(mui >= 0){
    return (catvar_t) mui;
  } else { //mui is the negative of a maximal index.
    catvar_t possibilities[len];

    unsigned top = 0;

    for(unsigned i = 0; i < len; i++){
      if(distributionClasses[i] == distributionClasses[-mui]){
        possibilities[top++] = (catvar_t)i;
      }
    }

    return (catvar_t) possibilities[random_cycle++ % top];
  }
}

//Regressor Ensemble

void RegressorEnsemble::train(Array<Sample> training){
  components.forEach<Array<Sample> >([](Regressor*& r, Array<Sample> t){r->train(t);}, training);
}

unsigned RegressorEnsemble::predictDistributionSize(){
  return components.length;
}

void RegressorEnsemble::loadDistributionSamples(Sample s, fracfloat_t* toWrite){
  for(unsigned i = 0; i < components.length; i++){
    toWrite[i] = components[i]->predict(s);
  }
}

//Classifier Ensemble

void ClassifierEnsemble::train(Array<Sample> training){
  components.forEach<Array<Sample> >([](Classifier*& c, Array<Sample> t){c->train(t);}, training);
}

unsigned ClassifierEnsemble::predictDistributionClassCount(){
  return classCount;
}

void ClassifierEnsemble::predictDistribution(Sample s, fracfloat_t* out){
  unsigned len = predictDistributionClassCount();
  arrayZero<fracfloat_t>(out, len); //Optimization: could set to -0.
  for(unsigned i = 0; i < components.length; i++){
    out[components[i]->predict(s)] += 1.0 / components.length;
  }
}