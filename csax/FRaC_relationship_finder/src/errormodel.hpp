#ifndef EMODEL_H
#define EMODEL_H

#include "sample.hpp"

#include "array.hpp"
#include "vectormath.hpp"


//Useful functions and consants
const fracfloat_t MAX_SURPRISAL = 16500; // we don't want infinite surprisal because then we can't rank test instances, so use a value such that 2^-s would underflow 
const fracfloat_t MAX_NORMALIZED_SURPRISAL = 16500; 

const fracfloat_t ln_2 = 0.69314718055994528622676398299518041312694549560546875;  // natural log, ln(2)=0.6931...
const fracfloat_t INV_SQRT_2_PI = 0.398942280401432702863218082711682654917240142822265625;

const fracfloat_t PI = 3.141592653589793115997963468544185161590576171875;
const fracfloat_t E = 2.718281828459045090795598298427648842334747314453125;

//Generic error model types

//Error model over type T
template <typename T> class ErrorModel{
  public:
      
  virtual fracfloat_t likelihood(T trueValue, T predictedValue) const {
    assert(0);
    return 0;
  }
  virtual fracfloat_t surprisal(T trueValue, T predictedValue) const {
    assert(0);
    return 0;
  }

  virtual void writeToStream(std::ostream& o) const = 0;
  
  //Architecture: Error model training function?
  
  virtual ~ErrorModel(){ }
  
  virtual void freeMemory(){
    
  }
  
  //friend std::ostream& operator<<(std::ostream& o, const ErrorModel<T>& s);
};

template<typename T> std::ostream& operator<<(std::ostream& o, const ErrorModel<T>& s){
  s.writeToStream(o);
  return o;
}


typedef ErrorModel<contvar_t> ContinuousErrorModel;
typedef ErrorModel<catvar_t> CategoricalErrorModel;
typedef ErrorModel<binvar_t> BinaryErrorModel;

//Distribution of real numbers.
//By subtracting real from predicted values, a real distribution can be used as an error model.
class RealDist : ContinuousErrorModel {
  public:

  //Likelihood functions (the value of the probability distribution function)
  fracfloat_t likelihood(fracfloat_t trueValue, fracfloat_t predictedValue) const;
  virtual fracfloat_t likelihood(fracfloat_t value) const = 0;
  
  //Log likelihood, for log space.  The door is open to implementers that wish to implement this directly.
  fracfloat_t logLikelihood(fracfloat_t trueValue, fracfloat_t predictedValue) const;
  //Virtual, with default implementation, because sometimes it's more accurate and/or faster to calculate this directly.
  virtual fracfloat_t logLikelihood(fracfloat_t value) const;

  //Convenience, surprisal uses logLikelihood function
  fracfloat_t surprisal(fracfloat_t trueValue, fracfloat_t predictedValue) const;
  fracfloat_t surprisal(fracfloat_t value) const;
  
  //A tricky way of approximating an integral of the form Integral of f(x) log(f(x)) over a probability distribution f.
  fracfloat_t approximateDifferentialEntropyFromSamples(Array<fracfloat_t> samples) const;
  fracfloat_t approximateDifferentialEntropyFromIntegral(fracfloat_t start, fracfloat_t end, unsigned count) const;
  //Approximate integral using standard numeric techniques.
  fracfloat_t integrate(fracfloat_t start, fracfloat_t end, unsigned count) const;
  
  //Functions necessary for error model scaling and such.
  virtual fracfloat_t maxValue() const{
    return likelihood(argMax());
  }
  virtual fracfloat_t argMax() const = 0;
  
  virtual void scaleHorizontal(fracfloat_t f){} //TODO can I mutually define these in terms of one another.
  virtual void scaleVertical(fracfloat_t f){
    scaleHorizontal(1 / f);
  }
  
  //Need to implement all continuous error model functions.
  virtual void writeToStream(std::ostream& o) const = 0;
  
  virtual ~RealDist() { }
  
  virtual void freeMemory() { }
  
};

template <typename CatModelTy> class BinaryErrorModelFromCategoricalErrorModel : public BinaryErrorModel{
  private:
  CatModelTy m;
  
  catvar_t binToCat(binvar_t in) const { 
    return ((in) ? 1 : 0);
  }

  public:
  BinaryErrorModelFromCategoricalErrorModel(CatModelTy cm) : m(cm) { }
  
  
  fracfloat_t likelihood(binvar_t trueValue, binvar_t predictedValue) const{
    return m.likelihood(binToCat(trueValue), binToCat(predictedValue));
  }
  
  fracfloat_t surprisal(binvar_t trueValue, binvar_t predictedValue) const{
    return m.surprisal(binToCat(trueValue), binToCat(predictedValue));
  }
  
  fracfloat_t normalizedSurprisal(binvar_t trueValue, binvar_t predictedValue) const{
    return m.normalizedSurprisal(binToCat(trueValue), binToCat(predictedValue));
  }
  
  void writeToStream(std::ostream& o) const {
    m.writeToStream(o);
  }
  
  void freeMemory(){
    m.freeMemory();
  }
};

class SurprisalMatrix;
class SurprisalHalfMatrix; //TODO implement this

//Optimization: implement these on their own for greater speed/less memory.
typedef BinaryErrorModelFromCategoricalErrorModel<SurprisalMatrix> BinarySurprisalMatrix;
typedef BinaryErrorModelFromCategoricalErrorModel<SurprisalHalfMatrix> BinarySurprisalHalfMatrix;


//Normally distributed gaussian error model.

// normal distribution
class Gaussian : public RealDist {
    public:

    //DATA
    fracfloat_t mu;
    fracfloat_t sigma;

    //LIFESPAN
    Gaussian(){}
    Gaussian(fracfloat_t mu, fracfloat_t sigma);
    Gaussian(const fracfloat_t *errors, unsigned n);
    Gaussian(const Array<fracfloat_t> errors);
    Gaussian(const fracfloat_t* trueValues, const fracfloat_t* predictedValues, unsigned n);

    //OPERATIONS
    fracfloat_t likelihood(fracfloat_t predoff) const;
    fracfloat_t logLikelihood(fracfloat_t x) const;
    
    fracfloat_t argMax() const{
      return mu;
    }
    
    void scaleHorizontal(fracfloat_t scaleFactor){
      mu *= scaleFactor;
      //TODO what happens to sigma?  Is this correct?
      sigma *= scaleFactor;
    }
    
    void writeToStream(std::ostream& o) const;
    
    private:
    
    void fitGaussian(const Array<fracfloat_t> errors);
};

class MultiGaussian : public RealDist {
  private:

  fracfloat_t entropy_value;
  Array<Gaussian> gaussians;

  public:
  
  MultiGaussian(){ }
  MultiGaussian(Array<Gaussian> data) : gaussians(data) {
    assert(!data.isEmpty());
  }
  
  static MultiGaussian fitGaussianKernelNBin(const fracfloat_t* trueValues, const fracfloat_t* predictedValues, unsigned count);
  static MultiGaussian fitGaussianKernelNBin(fracfloat_t* errors, unsigned count);
  
  
  static MultiGaussian fitGaussianKernel(const fracfloat_t* trueValues, const fracfloat_t* predictedValues, unsigned count, unsigned binCount);
  static MultiGaussian fitGaussianKernel(fracfloat_t* errors, unsigned count, unsigned binCount);
  
  fracfloat_t likelihood(fracfloat_t predoff) const;
  fracfloat_t logLikelihood(fracfloat_t x) const;
  
  
  fracfloat_t argMax() const{
    unsigned maxIndex = 0;
    fracfloat_t maxVal = likelihood(gaussians[0].mu);
    for(unsigned i = 1; i < gaussians.length; i++){
      fracfloat_t localVal = likelihood(gaussians[i].mu);
      if(localVal > maxVal){
        maxVal = localVal;
        maxIndex = i;
      }
    }
    return gaussians[maxIndex].mu;
    
    //TODO relying on the assumption that the max of a GKF is the max of a component.  This is not really the case...
  }
  
  //Optimization: Implement max

  void scaleHorizontal(fracfloat_t scaleFactor){
    gaussians.forEach<fracfloat_t>([](Gaussian& g, fracfloat_t sf){g.scaleHorizontal(sf);}, scaleFactor);
  }
  
  void writeToStream(std::ostream& o) const;
  
  void freeMemory(){
    gaussians.freeMemory();
  }
  
  private:
  void calculateEntropy(Array<fracfloat_t> errors);
};


class MultiGaussianWeighted : public RealDist {
  //private:
  public:
  fracfloat_t entropy_value;

  Gaussian* gaussians;
  fracfloat_t* weights;
  
  unsigned length;
  
  public:
  
  MultiGaussianWeighted(){ }
  
  MultiGaussianWeighted(Gaussian* gaussians, fracfloat_t* weights, unsigned length) : gaussians(gaussians), weights(weights), length(length) {
    assert(epsilonCompare<fracfloat_t>(sumTerms<fracfloat_t>(weights, length), (fracfloat_t) 1.0));
    assert(Array<fracfloat_t>(weights, length).conjunction([](fracfloat_t val){return val >= 0;}));
  }

  static MultiGaussianWeighted fitGaussianKernel(const fracfloat_t* trueValues, const fracfloat_t* predictedValues, unsigned sampleCount, unsigned binCount);
  static MultiGaussianWeighted fitGaussianKernel(fracfloat_t* errors, unsigned sampleCount, unsigned binCount);
  
  fracfloat_t likelihood(fracfloat_t predoff) const;
  fracfloat_t logLikelihood(fracfloat_t x) const;

  fracfloat_t argMax() const{
    unsigned maxIndex = 0;
    fracfloat_t maxVal = likelihood(gaussians[0].mu);
    for(unsigned i = 1; i < length; i++){
      fracfloat_t localVal = likelihood(gaussians[i].mu);
      if(localVal > maxVal){
        maxVal = localVal;
        maxIndex = i;
      }
    }
    return gaussians[maxIndex].mu;
    
    //TODO relying on the assumption that the max of a GKF is the max of a component.  This is not really the case...
  }

  void scaleHorizontal(fracfloat_t scaleFactor){
    Array<Gaussian>(gaussians, length).forEach<fracfloat_t>([](Gaussian& g, fracfloat_t sf){g.scaleHorizontal(sf);}, scaleFactor);
  }
  
  void writeToStream(std::ostream& o) const;
  
  void freeMemory(){
    delete [] gaussians;
    delete [] weights;
  }
  
};



template<typename ModelTy> class ErrorModelEnsemble : public ErrorModel<ModelTy> {
  Array<ErrorModel<ModelTy> > models;
  
  ErrorModelEnsemble(){ }
  ErrorModelEnsemble(Array<ErrorModel<ModelTy> > models) : models(models) { }

  fracfloat_t likelihood(fracfloat_t predoff) const;
  fracfloat_t likelihood(fracfloat_t trueValue, fracfloat_t predictedValue) const;
  fracfloat_t surprisal(fracfloat_t trueValue, fracfloat_t predictedValue) const;

  fracfloat_t logLikelihood(fracfloat_t x) const;

  fracfloat_t normalizedSurprisal(fracfloat_t trueValue, fracfloat_t predictedValue) const;
  
  void writeToStream(std::ostream& o) const;
  
  void freeMemory(){
    models.forEach([](ErrorModel<ModelTy> model){model.freeMemory();});
    models.freeMemory();
  }
};

//Confusion matrix error model

class SurprisalMatrix : public CategoricalErrorModel{
  private:
    //TODO don't keep pMatrix, just take e^(-x) to calculate p
    fracfloat_t* surprisalMatrix;
    unsigned classCount;

    fracfloat_t* sMatrixCellLookup(catvar_t trueClass, catvar_t predictedClass) const;
    fracfloat_t pMatrixCellValueLookup(catvar_t trueClass, catvar_t predictedClass) const;

    void initializeMatrixSeeded(fracfloat_t seed);
    void probabilityMatrixToSurprisalMatrix();

    public:
    SurprisalMatrix(){}
    SurprisalMatrix(unsigned classCount, catvar_t* trueClasses, catvar_t* predictedClasses, unsigned instanceCount);

    fracfloat_t surprisal(catvar_t trueClass, catvar_t predictedClass) const;

    void writeToStream(std::ostream& o) const;
    
    void freeMemory(){
      delete [] surprisalMatrix;
    }
};

#endif
