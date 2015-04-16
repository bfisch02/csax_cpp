
#include <math.h>

const char *distance_function_name() { return "Euclidean"; }

double distance(const double *x, const double *y, const int dim) { 
	int i;
	double ssd = 0.0;
	for (i=0; i<dim; i++) { 
		double diff = x[i] - y[i];
		ssd += diff * diff;
	}
	return sqrt(ssd);
}

