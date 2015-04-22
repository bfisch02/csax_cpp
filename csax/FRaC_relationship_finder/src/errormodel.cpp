#include "errormodel.hpp"
#include "vectormath.hpp"

#include <math.h>

#include <assert.h>

/////////////////////
//Real Distribution//
/////////////////////

fracfloat_t RealDist::likelihood(fracfloat_t trueValue, fracfloat_t predictedValue) const {
  return likelihood(predictedValue - trueValue);
}

fracfloat_t RealDist::logLikelihood(fracfloat_t trueValue, fracfloat_t predictedValue) const {
  return logLikelihood(predictedValue - trueValue);
}
fracfloat_t RealDist::logLikelihood(fracfloat_t value) const {
  return log2ff(likelihood(value));
}

fracfloat_t RealDist::surprisal(fracfloat_t trueValue, fracfloat_t predictedValue) const {
  return surprisal(predictedValue - trueValue);
}
fracfloat_t RealDist::surprisal(fracfloat_t value) const {
  return - logLikelihood(value);
}

fracfloat_t RealDist::approximateDifferentialEntropyFromSamples(Array<fracfloat_t> samples) const {
  assert(log2ff(FRACFLOAT_EPSILON) > FRACFLOAT_NEGATIVE_INFINITY);
  fracfloat_t sum = 0;
  for(unsigned i = 0; i < samples.length; i++){
    fracfloat_t ll = logLikelihood(samples[i]);
    //assert(epsilonCompare(ll, log2ff(likelihood(samples[i])))); //This won't be true if likelihood is 0.
    if(FRACFLOAT_NEGATIVE_INFINITY < ll ) sum += ll;
    //Otherwise, the sample is too unlikely (or NaN), consider it a 0 log 0 situation.
  }

  sum /= samples.length;
  return -sum;
}

fracfloat_t RealDist::approximateDifferentialEntropyFromIntegral(fracfloat_t start, fracfloat_t end, unsigned count) const {
  assert(count > 1);
  fracfloat_t sum = 0;
  for(unsigned i = 0; i < count; i++){
    fracfloat_t l = likelihood(start + i * (end - start) / (count - 1)); // count - 1 to avoid left bias.
    if(l > FRACFLOAT_EPSILON) sum += l * log2ff(l);
    //Otherwise, 0 log 0 = 0 for entropy calculation.
  }
  
  sum *= (end - start) / (count - 1); //Multiply by the width of each rectagle.
  return -sum;
}

fracfloat_t RealDist::integrate(fracfloat_t start, fracfloat_t end, unsigned count) const {
  assert(count > 1);
  fracfloat_t sum = 0;
  for(unsigned i = 0; i < count; i++){
    fracfloat_t l = likelihood(start + i * (end - start) / (count - 1)); // count - 1 to avoid left bias.
    if(l > FRACFLOAT_EPSILON) sum += l;
    //Otherwise, 0 log 0 = 0 for entropy calculation.
  }
  
  sum *= (end - start) / (count - 1); //Multiply by the width of each rectagle.
  return sum;
}

////////////
//GAUSSIAN//
////////////

//////////////
//CONSTRUCTORS

Gaussian::Gaussian(fracfloat_t mu, fracfloat_t sigma) : mu(mu), sigma(sigma) { }

Gaussian::Gaussian(const fracfloat_t *errors, unsigned n){
  fitGaussian(Array<fracfloat_t>(n, (fracfloat_t*) errors)); //Remove const on errors via cast (we don't modify it, don't worry!)
}

Gaussian::Gaussian(const Array<fracfloat_t> errors) : Gaussian(errors.data, errors.length) { }

Gaussian::Gaussian(const fracfloat_t* trueData, const fracfloat_t* predictedData, unsigned count){
  fracfloat_t differences[count];
  for(unsigned i = 0; i < count; i++){
    differences[i] = predictedData[i] - trueData[i];
  }
  
  fitGaussian(Array<fracfloat_t>(differences, count));
}

void Gaussian::fitGaussian(const Array<fracfloat_t> errors){
  mu = mean<fracfloat_t>(errors);
  sigma = stdev<fracfloat_t>(errors, mu) + FRACFLOAT_EPSILON;
}

fracfloat_t Gaussian::likelihood(fracfloat_t predOff) const {
	fracfloat_t diff = mu - predOff;
	fracfloat_t lval = (INV_SQRT_2_PI / sigma) * exp( -(diff*diff) / (2 * sigma * sigma) );
  return lval < FRACFLOAT_EPSILON ? FRACFLOAT_EPSILON : lval;
}

//For numerics purposes: very bad things happen with floats when we take the log of the likelihood directly. 
fracfloat_t Gaussian::logLikelihood(fracfloat_t predOff) const{
  fracfloat_t diff = mu - predOff;
  return INV_LN_2 * (log(INV_SQRT_2_PI / sigma) + (-(diff * diff) / (2 * sigma * sigma) ));
}

void Gaussian::writeToStream(std::ostream& o) const{
  o.precision(3);
  o << "{" << "Gaussian:" << " " << "mu = " << mu << " sigma = " << sigma << " " << "}";
}

std::ostream& operator<<(std::ostream& o, const Gaussian& s)
{
  s.writeToStream(o);
  return o;
}

////////////////
//MULTI GAUSSIAN


MultiGaussian MultiGaussian::fitGaussianKernelNBin(const fracfloat_t* trueValues, const fracfloat_t* predictedValues, unsigned count){
  fracfloat_t errors[count];
  for(unsigned i = 0; i < count; i++){
    errors[i] = predictedValues[i] - trueValues[i];
  }
  return fitGaussianKernelNBin(errors, count);
}

MultiGaussian MultiGaussian::fitGaussianKernelNBin(fracfloat_t* errors, unsigned count){
  fracfloat_t sigma = 1.0 / sqrt(count);
  Array<Gaussian> gaussians = Array<Gaussian>(count);

  for(unsigned i = 0; i < count; i++){
    gaussians[i] = Gaussian(errors[i], sigma);
  }
  
  return MultiGaussian(gaussians);
  
}


MultiGaussian MultiGaussian::fitGaussianKernel(const fracfloat_t* trueValues, const fracfloat_t* predictedValues, unsigned count, unsigned binCount){
  fracfloat_t errors[count];
  for(unsigned i = 0; i < count; i++){
    errors[i] = predictedValues[i] - trueValues[i];
  }
  return fitGaussianKernel(errors, count, binCount);
}

MultiGaussian MultiGaussian::fitGaussianKernel(fracfloat_t* errors, unsigned count, unsigned binCount){
  //Optimization: could use kth order statistics.
  //Note: Parallel was causing problems.
  std::sort(errors, errors + count);
  
  Array<Gaussian> gaussians = Array<Gaussian>(binCount);
  
  for(unsigned i = 0; i < binCount; i++){
    unsigned bin0 = count * i / binCount;
    unsigned bin1 = count * (i + 1) / binCount - 1;

    //Another way to do it:
    //Gaussian centered at the center of the bin, stdev the width of the bin.
    //Detail should stdev be half the width?

    gaussians[i] = Gaussian( 0.5 * (errors[bin0] + errors[bin1]), 1.0 * (errors[bin1] - errors[bin0]) + FRACFLOAT_EPSILON);
    //gaussians[i] = Gaussian(mean<fracfloat_t>(errors + bin0, bin1 - bin0), errors[bin1] - errors[bin0] + FRACFLOAT_EPSILON);
  }

  return MultiGaussian(gaussians);
}

fracfloat_t MultiGaussian::likelihood(fracfloat_t error) const {
  fracfloat_t sum = 0;
  for(unsigned i = 0; i < gaussians.length; i++){
    sum += gaussians[i].likelihood(error); //This is the log likelihood.  We implicitly have likelihood by the input distribution (it is weighted more heavily toward more likely events).
  }
  sum /= gaussians.length; //This normalizes so the total probability is 1.0
  return sum;
}
fracfloat_t MultiGaussian::logLikelihood(fracfloat_t x) const {
  return log2ff(likelihood(x));  //Optimization: Can clever tricks like in Gaussian be used here?
}

void MultiGaussian::writeToStream(std::ostream& o) const {
  o << "{Multi Gaussian: {";
  o << gaussians[0];
  for(unsigned i = 1; i < gaussians.length; i++){
    o << ", " << gaussians[i];
  }
  o << "}}";
}

/////////////////////////
//Weighted Multi Gaussian

MultiGaussianWeighted MultiGaussianWeighted::fitGaussianKernel(const fracfloat_t* trueValues, const fracfloat_t* predictedValues, unsigned count, unsigned binCount){
  fracfloat_t errors[count];
  for(unsigned i = 0; i < count; i++){
    errors[i] = predictedValues[i] - trueValues[i];
  }
  return fitGaussianKernel(errors, count, binCount);
}

MultiGaussianWeighted MultiGaussianWeighted::fitGaussianKernel(fracfloat_t* errors, unsigned count, unsigned binCount){
//  std::sort(errors, errors + count);
  
  fracfloat_t minVal = min<fracfloat_t>(errors, count);
  fracfloat_t maxVal = max<fracfloat_t>(errors, count);
  
  fracfloat_t binWidth = (fracfloat_t)((maxVal - minVal) / binCount);
  
  
  //Calculate the weight of each bin
  fracfloat_t* weights = new fracfloat_t[binCount];
  
  arrayZero<fracfloat_t>(weights, binCount);
  
  for(unsigned i = 0; i < count; i++){
    //Find the bin
    unsigned bin = (unsigned) ((errors[i] - minVal) / binWidth);
    if(bin == binCount) bin = binCount - 1; //To handle the top bin
    assert(bin < binCount);
    weights[bin] += (fracfloat_t) (1.0 / count);
  }
  assert(weights[0] > 0); //First bin always has at least the min sample.
  //assert(weights[binCount - 1] > 0); //Last bin always has at least the max sample.  (This is only true when minVal != maxVal)
  
  //Make gaussians (stdev = bin width)
  Gaussian* gaussians = new Gaussian[binCount];
  
  for(unsigned i = 0; i < binCount; i++){
    gaussians[i] = Gaussian(minVal + binWidth * i + binWidth / 2, binWidth);
  }

  MultiGaussianWeighted mg = MultiGaussianWeighted(gaussians, weights, binCount);
  return mg;
}

fracfloat_t MultiGaussianWeighted::likelihood(fracfloat_t error) const {
  fracfloat_t sum = 0;
  for(unsigned i = 0; i < length; i++){
    sum += weights[i] * gaussians[i].likelihood(error);
  }
  return sum;
}

fracfloat_t MultiGaussianWeighted::logLikelihood(fracfloat_t x) const {
  return log2(likelihood(x));  //Optimization: Can clever tricks like in Gaussian be used here?
}

void MultiGaussianWeighted::writeToStream(std::ostream& o) const {
  o << "{Multi Gaussian Weighted: {";
  o << weights[0] << " * " << gaussians[0];
  for(unsigned i = 1; i < length; i++){
    o << ", " << weights[i] << " * " << gaussians[i];
  }
  o << "}}";
}


/////////////////////////////
//Normalized Surprisal Matrix

//Normalized Surprisal matrix constructor.  The constructor is given classifications and creates a matrix of surprisal scores.
SurprisalMatrix::SurprisalMatrix(unsigned classCount, catvar_t* trueClasses, catvar_t* predictedClasses, unsigned instanceCount) : classCount(classCount) {
	
	//A roundabout technique is used to calculate the surprisal matrix.  Intuitively, one would:
	/*
		1.  Create an n x n matrix and initialize to 0
		2.  Increment the [true,predicted] cell for each classification instance
		3.  Apply smoothing by adding 1 to each cell of the matrix
		4.  Normalize the matrix such that each row sums to 1
		5.  Calculate the entropy of the input distribution
		6.  Calculate the surprisal for each cell
		7.  Calculate the normalized surprisal for each cell by subtracting entropy from surprisal scores
	*/
	
	//However, a more computationally efficient method exists!
	/*
		1.  Create an n x n matrix and initialize each cell to the smoothing factor, 1
		2.  Increment the [true,predicted] cell for each classification instance
		3.  Calculate the normalized surprisal, one row at a time:
			a.  Calculate the sum of the row
			b.  Calculate the surprisal value for each cell by taking the log of the cell over the sum (this folds in the normalization step)
			c.  Calculate the normalized surprisal value for each cell by subtracting the entropy from the cell
	*/
	
	/*
	//Because of the way calculations are organized, the standard confusion matrix ordering is logical.
	//Hexidecimal numbers represent indices in the underlying array.
	//This is hidden away by the use of the matrixCellLookup function.
		
	THIS:                                  NOT THIS:
	            predicted class             true class
	           +----                       +----
	true class |0123             predicted |0123
	           |4567               class   |4567
	           |89ab                       |89ab
	           |cdef                       |cdef
	
	//This allows the lookup of surprisals for different predicted classes to be more performant cachewise than the transpose.
	
	*/
	
	//This technique leaves a matrix of surprisal scores, minimizes costly log operations, and requires only constant memory overhead.
	//Noticeable numerics issues may result, particularly for large matrices with large test datasets.  
  
  fracfloat_t seedVal = 1.0; //Should probably be parametrized.
  initializeMatrixSeeded(seedVal);
	
	//Load the predictions into the matrix
	for(unsigned i = 0; i < instanceCount; i++){
		*sMatrixCellLookup(trueClasses[i], predictedClasses[i]) += 1;
	}
	
  //Make sure the matrix has the correct total value
  assert(epsilonCompare<fracfloat_t>(seedVal * classCount * classCount + instanceCount, sumTerms<fracfloat_t>(surprisalMatrix, classCount * classCount)));
  
  probabilityMatrixToSurprisalMatrix();
}

//Allocate a new matrix and set all cells to the given seed.
void SurprisalMatrix::initializeMatrixSeeded(fracfloat_t seed){
	surprisalMatrix = new fracfloat_t[classCount * classCount];
	//Initialize the matrix and apply smoothing.
	arraySet<fracfloat_t>(surprisalMatrix, classCount * classCount, seed);
}

void SurprisalMatrix::probabilityMatrixToSurprisalMatrix(){
  
  //Normalization by row (true class)
#if 0
	//Calculate normalized surprisal, one row at a time
	for(unsigned trueClass = 0; trueClass < classCount; trueClass++){
		
		//Calculate the total for this predicted class (for cellwise normalization)
		fracfloat_t thisTrueClassTotal = 0;
		for(unsigned predictedClass = 0; predictedClass < classCount; predictedClass++){
			thisTrueClassTotal += *sMatrixCellLookup(trueClass, predictedClass);
		}
    
    for(unsigned predictedClass = 0; predictedClass < classCount; predictedClass++){
		//Now, surprisal is just -log_2(p).  Calculate it for each cell
			fracfloat_t p = *sMatrixCellLookup(trueClass, predictedClass) / thisTrueClassTotal; //P(Predicted predictedClass | True class trueClass)
			fracfloat_t cellSurprisal = -log2ff(p);
      
			*sMatrixCellLookup(trueClass, predictedClass) = cellSurprisal;
      *pMatrixCellValueLookup(trueClass, predictedClass) = p;
			
			assert(p > 0 && p < 1); //Note: This assertion may fail (only) if pseudocount is 0.
		}
	}
#endif
  
  //Normalization by column (predicted class)
  
	//Calculate normalized surprisal, one row at a time
	for(unsigned predictedClass = 0; predictedClass < classCount; predictedClass++){
		
		//Calculate the total for this predicted class (for cellwise normalization)
		fracfloat_t thisTrueClassTotal = 0;
		for(unsigned trueClass = 0; trueClass < classCount; trueClass++){
			thisTrueClassTotal += *sMatrixCellLookup(trueClass, predictedClass);
		}
    
    fracfloat_t distSum = 0;
    for(unsigned trueClass = 0; trueClass < classCount; trueClass++){
		//Now, surprisal is just -log_2(p).  Calculate it for each cell
			fracfloat_t p = *sMatrixCellLookup(trueClass, predictedClass) / thisTrueClassTotal; //P(Predicted predictedClass | True class trueClass)
			fracfloat_t cellSurprisal = -log2ff(p);
      
			*sMatrixCellLookup(trueClass, predictedClass) = cellSurprisal;
			
			assert(p > 0 && p < 1); //Note: This assertion may fail (only) if pseudocount is 0.
      distSum += p;
		}
    assert(epsilonCompare<fracfloat_t>(distSum, 1));
	}
}

fracfloat_t* SurprisalMatrix::sMatrixCellLookup(catvar_t trueClass, catvar_t predictedClass) const {
  assert(trueClass < classCount);
  assert(predictedClass < classCount);
	return surprisalMatrix + (classCount * trueClass + predictedClass);
}

fracfloat_t SurprisalMatrix::pMatrixCellValueLookup(catvar_t trueClass, catvar_t predictedClass) const {
	return exp2ff(- *sMatrixCellLookup(trueClass, predictedClass));
}

fracfloat_t SurprisalMatrix::surprisal(catvar_t trueClass, catvar_t predictedClass) const {
  return *sMatrixCellLookup(trueClass, predictedClass);
}

void SurprisalMatrix::writeToStream(std::ostream& o) const{
  o.precision(3);
  o << "Surprisal Matrix:" << std::endl;
  for(unsigned i = 0; i < classCount; i++){
    o << "\t" << i;
  }
  o << std::endl;
  for(unsigned i = 0; i < classCount; i++){
    o << i;
    for(unsigned j = 0; j < classCount; j++){
      o << "\t" << *sMatrixCellLookup(i, j);
    }
    o << std::endl;
  }
  o << "Confusion Matrix:" << std::endl;
  for(unsigned i = 0; i < classCount; i++){
    o << "\t" << i;
  }
  o << std::endl;
  for(unsigned i = 0; i < classCount; i++){
    o << i;
    for(unsigned j = 0; j < classCount; j++){
      o << "\t" << pMatrixCellValueLookup(i, j);
    }
    o << std::endl;
  }
}