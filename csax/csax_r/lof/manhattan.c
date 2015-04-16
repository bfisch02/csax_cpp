#include <math.h>

const char* distance_function_name() { return "L-1 Norm (Manhattan)"; }

double distance(const double *x, const double *y, const int dim) { 
	int i;
	double sum = 0.0;
	for (i=0; i<dim; i++) { 
		sum += fabs(x[i] - y[i]);
	}
	return sum;
}

