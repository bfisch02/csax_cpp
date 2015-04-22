#ifndef MLTYPES_H
#define MLTYPES_H

#include <assert.h>
#include <type_traits>

#include "sample.hpp"
#include "errormodel.hpp"

#include "array.hpp"
#include "vectormath.hpp"

//Classes for abstract ML types.
template <typename A> class MlObject{
  public:
  virtual A predict(Sample) = 0;
  
  virtual void train(Array<Sample> training){ } //The default is to do nothing.
  
  std::string predTyName() const{
    if(std::is_same<A,contvar_t>::value){
      return "Regressor";
    }
    else if(std::is_same<A,catvar_t>::value){
      return "Classifier";
    }
    else if(std::is_same<A,binvar_t>::value){
      return "Binary Classifier";
    }
    return "Unknown Predictor";
  }
  
  virtual void writeToStream(std::ostream& o) const{
    o << "{" << predTyName() << "}";
  }
  
  virtual void freeMemory(){ }
  
  virtual ~MlObject(){ }
};

template <typename A> std::ostream& operator<<(std::ostream& o, const MlObject<A>& s){
  s.writeToStream(o);
  return o;
}

typedef MlObject<contvar_t> Regressor;
typedef MlObject<catvar_t> Classifier;
typedef MlObject<binvar_t> BinaryClassifier;

//Constant Predictors:
// These are useful as very simple predictors, and can be used in special case decision trees where the same item is always outputted.

template<typename A> class ConstantMlObject : public MlObject<A>{
  private:
  A constant;
  public:
  ConstantMlObject(A constant){
    this->constant = constant;
  }
  
  A predict(Sample s){
    return constant;
  }
  
  void writeToStream(std::ostream& o) const{
    o << "{Constant " << MlObject<A>::predTyName() << " { " << constant << " }" << "}";
  }
};

typedef ConstantMlObject<contvar_t> ConstantRegressor;
typedef ConstantMlObject<catvar_t> ConstantClassifier;
typedef ConstantMlObject<binvar_t> ConstantBinaryClassifier;

///////////////////////////
//Probabalistic ML objects:

class ProbabilisticRegressor : public Regressor{
  public:
  
  //Architecture: Consider default implementation in terms of gaussian
  virtual unsigned predictDistributionSize() = 0; //Architecture: Should this take a Sample?  Then things could filter.
  virtual void loadDistributionSamples(Sample s, fracfloat_t* toWrite) = 0;

  virtual Array<fracfloat_t> predictDistributionSamples(Sample s){
      unsigned size = predictDistributionSize();
      fracfloat_t* data = new fracfloat_t[size];
      loadDistributionSamples(s, data);
      return Array<fracfloat_t>(data, size);
  }
  
  Gaussian predictDistributionGaussian(Sample s){
    //Optimization: Use above function.
    unsigned size = predictDistributionSize();
    fracfloat_t mem[size];
    loadDistributionSamples(s, mem);
    Gaussian g = Gaussian(mem, size);
    return g;
  }
  
  fracfloat_t predict(Sample s){
    //Prediction is the mean.  This is so that the gaussian fit to the distribution has the same mean as the predict function.
    //Otherwise, median may make more sense.
    unsigned size = predictDistributionSize();
    fracfloat_t mem[size];
    loadDistributionSamples(s, mem);
    return mean<fracfloat_t>(mem, size);
  }
};

extern unsigned random_cycle;

class ProbabilisticClassifier : public Classifier
{
  public:

  virtual unsigned predictDistributionClassCount() = 0;
  virtual void predictDistribution(Sample s, fracfloat_t* out) = 0;
  virtual void train(Array<Sample>) = 0;
  
  virtual void writeToStream(std::ostream&) const = 0;
  
  /*
  fracfloat_t* predictDistribution(Sample s){
    assert(featureIndex < st->catvarCount);
    fracfloat_t* out = new fracfloat_t[st->catvarSizes[featureIndex]];
    predictDistribution(st, featureIndex, s, out);

    //#ifdef DEBUG
    fracfloat_t sum = 0;
    for(unsigned i = 0; i < featureIndex; i++){
      assert(out[i] >= 0 && out[i] <= 1);
      sum += out[i];
    }
    assert((sum < .001) && (sum > -.001));
    //#endif

    return out;
    
    //OVERRIDE IN PROBCLASS
  }
  */
  
  catvar_t predict(Sample s);
  
};

class ProbabilisticBinaryClassifier : public BinaryClassifier{
  public:
  
  virtual fracfloat_t predictDistribution(Sample s) = 0;
  
};

//Architecture: used to make an ensemble.
//For use in constructors.
template<typename T> MlObject<T>* takeBest(Array<MlObject<T> > inObj, Array<Sample> val){
  //TODO implementation forthcoming.
  return NULL;
}

class RegressorEnsemble : public ProbabilisticRegressor{
  private:
  Array<Regressor*> components;
  //Architecture: Also take probabilistic regressors.
  public:
  
  RegressorEnsemble(Array<Regressor*> r) : components(r) { }
      
  unsigned predictDistributionSize();
  void loadDistributionSamples(Sample s, fracfloat_t* toWrite);
  void train(Array<Sample> training);
  
  void freeMemory(){
    components.forEach([](Regressor*& r){r->freeMemory(); delete r;});
    components.freeMemory();
  }
  
  void writeToStream(std::ostream& o) const{
    o << "{Regressor Ensemble:{"; //Detail: Name the feature?
    for(unsigned i = 0; i < components.length; i++){
      o << components[i];
    }
    o << "}}";
  }
};

class ClassifierEnsemble : public ProbabilisticClassifier{
  private:
  Array<Classifier*> components;
  unsigned classCount;
  //Architecture: Also take probabilistic classifiers.
  
  public:
  ClassifierEnsemble(Array<Classifier*> components, SampleStructure ss, unsigned featureIndex) : components(components) {
    classCount = ss.catvarSizes[featureIndex];
  }
  
  void train(Array<Sample>);
  
  unsigned predictDistributionClassCount();
  void predictDistribution(Sample s, fracfloat_t* out);
  
  void freeMemory(){
    components.forEach([](Classifier*& c){c->freeMemory(); delete c;});
    components.freeMemory();
  }
  
  void writeToStream(std::ostream& o) const{
    o << "{Classifier Ensemble:{";
    //TODO
    //components.forEach<std::ostream&>([](Classifier*& c, std::ostream& o){o << "{" << *c << "}";}, o);
    o << "}}";
  }
};

class BinaryClassifierEnsemble : public BinaryClassifier{
  private:
  Array<BinaryClassifier*> components;
  public:
  binvar_t predict(Sample);
  
  void freeMemory(){
    components.forEach([](BinaryClassifier*& bc){bc->freeMemory(); delete bc;});
    components.freeMemory();
  }
  
  void writeToStream(std::ostream& o) const{
    o << "{Binary Classifier Ensemble:{";
    //TODO
    //components.forEach<std::ostream&>([](BinaryClassifier*& bc, std::ostream& o){o << "{" << *bc << "}";}, o);
    o << "}}";
  }
};

#endif
