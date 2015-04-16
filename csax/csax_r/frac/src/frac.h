#ifndef FRAC_H
#define FRAC_H
#include <math.h> // log, logl

// noto:  define precision of probability and surprisal-based calculations 
const double MAX_SURPRISAL = 16500; // we don't want infinite surprisal because then we can't rank test instances, so use a value such that 2^-s would underflow 
const double MAX_NORMALIZED_SURPRISAL = 16500; 
const double ln_2 = 0.69314718055994528622676398299518041312694549560546875;  // natural log, ln(2)=0.6931...
#define lg(x) (log(x)/ln_2)

// normal distribution
struct gaussian {
    double mu;
    double sigma;
};

// noto, use [modified] Gaussian error model
typedef gaussian error_model; 

// compute entropy of Gaussian or sample modeled by gaussian
double entropy(const gaussian &g); 
double entropy(const double *X, const int n);

// model errors (as Gaussian)
error_model model_error(const double *errors, int n);

// compute likelihood of error according to the model
double likelihood(double error, const error_model &E);

#endif
