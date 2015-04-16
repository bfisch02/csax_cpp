#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
	
typedef struct { 
	int kLB,kUB; // k (MinPts) lower- and upper-bound
	int F; // number of features
	int T; // number of training instances
	int M; // total number of instances

	int C; // number of examples for which to compute score
	int C0; // first example for which to compute score
	int N; // number of neighbors

}parameter_set;

// for printing paragraphs
void hang(FILE *out, const char *msg);

// Parse command-line arguments.  Exit if input is wrong.
parameter_set parse_command_line(const int argc, const char **argv, const char *SYNOPSIS, const char *USAGE);

// subroutine to read input data and compute distance.  triangular matrix
// returned must be freed by user.
double** compute_distance(const parameter_set *param, FILE *verbose);

