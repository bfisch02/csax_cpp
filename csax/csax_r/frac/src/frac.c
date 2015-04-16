#include "frac.h"
#include <math.h>
//#include <assert.h>
#include <stdio.h> // warning/error messages


// Compute sample mean/stdev
gaussian model_sample( const double *X, int n ) {

	int i;
	double sum = 0, ssd = 0;
	gaussian g;

	for (i=0; i<n; i++) { sum += X[i]; }
	g.mu = sum / n;

	for (i=0; i<n; i++) { 
		double diff = g.mu - X[i];
		ssd += diff*diff;
	}
	g.sigma = sqrt( ssd / (n-1) ); // using Bessel's correction

	return g;
}

// compute error model from two lists
error_model model_error(const double *errors, int n) { return model_sample(errors,n); }

// compute likelihood of error according to the model
// em: pointer to error model (Gaussian, with mu and sigma)
// y: pointer to single observed value 
// g: pointer to single predicted value (passed as pointer so that NULL value indicates missing value--so compute likelihood of y only)
double likelihood(double x, const gaussian &g) { 
	
	static const double INV_SQRT_2_PI = 0.398942280401432702863218082711682654917240142822265625;
	double diff = g.mu  -  x;
	return (INV_SQRT_2_PI / g.sigma) * exp( -(diff*diff) / (2 * g.sigma * g.sigma) );

}

// compute the entropy of the error model
double entropy( const gaussian &g ) {
	static const double PI = 3.141592653589793115997963468544185161590576171875;
	static const double E = 2.718281828459045090795598298427648842334747314453125;
	return 0.5 * lg(2 * PI * E * g.sigma * g.sigma);
}
double entropy(const double *X, int n) { return entropy(model_sample(X,n)); }

