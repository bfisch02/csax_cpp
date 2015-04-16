#include "oflib.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
	
// *The* distance function
extern double distance(const double *x, const double *y, const int dim);

// for printing paragraphs
void hang(FILE *out, const char *msg) { 
	const int TW=80; // line width
	int c=0;
	const char *p;
	for (p=msg; *p != '\0'; p++) { 
		if (c > 0 || *p > ' ') { fprintf(out,"%c",*p); c++; } // don't print spaces to start a line
		if (c > TW && *p <= ' ') { fprintf(out,"\n"); c=0; } // wait for a space to end a line
	}
}

// Parse command-line arguments.  Exit if input is wrong.
parameter_set parse_command_line(const int argc, const char **argv, const char *SYNOPSIS, const char *USAGE) {

	// Arguments are:
	//	1	k lower bound (i.e. MinPtsLB)
	//	2	k upper bound
	//	3	no. features
	//	4	no. training instances	
	//	5	no. testing instances	 (optional)

	if (argc!=5 && argc!=6) {
		
		fprintf(stderr, "\n\n"); 
		hang(stderr, SYNOPSIS);
		fprintf(stderr, "\n\n"); 
		fprintf(stderr, "%s", USAGE);
		fprintf(stderr, "\n\n"); 
		exit(-1);

	}

	parameter_set param;

	const int UNSUPERVISED = (argc==5); // i.e., number of test examples not supplied

	param.F = atoi(argv[3]);		// num features
	param.T = atoi(argv[4]);		// training instances
	param.M = UNSUPERVISED ? param.T : param.T + atoi(argv[5]); // total instances
	
	if (param.F <= 0) { fprintf(stderr,"ERROR: number of features must be greater than 0 (F=%d)\n", param.F); exit(-1); }
	if (param.M <= 0) { fprintf(stderr,"ERROR: number of instances must be greater than 0 (M=%d)\n", param.M); exit(-1); }

	int user_MinPtsLB = atoi(argv[1]); 
	if (user_MinPtsLB < 2) { fprintf(stderr,"Setting MinPts lower bound <- 2 (%i is too small)\n", user_MinPtsLB); user_MinPtsLB=2; }
	if (user_MinPtsLB >= param.T) { fprintf(stderr,"Setting MinPts lower bound <- %i (%i is too large)\n", param.T-1, user_MinPtsLB); user_MinPtsLB=param.T-1; }
	int user_MinPtsUB = atoi(argv[2]); 
	if (user_MinPtsUB < 2) { fprintf(stderr,"Setting MinPts upper bound <- 2 (%i is too small)\n", user_MinPtsUB); user_MinPtsUB=2; }
	if (user_MinPtsUB >= param.T) { fprintf(stderr,"Setting MinPts upper bound <- %i (%i is too large)\n", param.T-1, user_MinPtsUB); user_MinPtsUB=param.T-1; }
	
	param.kLB = user_MinPtsLB;
	param.kUB = user_MinPtsUB;

	// The following change standard LOF to the special version we use to
	// estimate LOF over a train and test set.  The basic idea is that test
	// examples are "invisible" to everybody else so that each test can be
	// calculated independently.
	
	// M = total instances
	// T = training instances

	// To make this 'standard' LOF, which calculates a score for all input
	// examples, set C0 = 0, C = N = M.

	param.N  = param.T;                                  // number of available neighbors
	param.C  = UNSUPERVISED ? param.M : param.M-param.T; // number of examples for which to calculate LOF
	param.C0 = UNSUPERVISED ? 0 : param.T;               // 1st example for which to calculate LOF

	return param; 
}
	

// subroutine to read input data and compute distance
double** compute_distance(const parameter_set *param, FILE *verbose) {

	const int T = param->T;
	const int M = param->M; const int N = param->N; const int F = param->F;

	int i,j,f;
	time_t t0;
	const int vperiod = T/20 + 100 - (T/20)%100;	// adjust to a percentage of training set, round to nearest 100

	// allocate space for data (examples x features)
	double **data = (double**)malloc(sizeof(double*) * M);	// freed after stored_dist is calculated
	for (i=0; i<M; i++) { data[i] = (double*)malloc(sizeof(double)*F); }

	// space for pairwise distance (need only lower triangle of training x
	// training instances, and distance from test instances to training
	// instances)
	double **stored_dist = (double**)malloc(sizeof(double*) * M); 	// freed at the end
	for (i=0; i<T; i++) { stored_dist[i] = (double*)malloc(sizeof(double)*i); } // must be i > j
	for (i=T; i<M; i++) { stored_dist[i] = (double*)malloc(sizeof(double)* (N < i ? N : i)); }
	
	// read data
	if (verbose) { fprintf(verbose, "# Read input...\n"); }
	FILE *in = stdin; 
	for (i=0; i<M; i++) { 
		for (f=0; f<F; f++) { 
			fscanf(in, "%lf,", data[i]+f); 
		}
	}
	fclose(in);

	t0 = time(NULL);
	if (verbose) { fprintf(verbose, "# Compute pairwise distance for %i training instances...",  T); }
	for (i=0; i<T; i++) { 
		if (verbose && !(i%vperiod)) { fprintf(verbose, "."); }
		for (j=0; j<i; j++) { 
			stored_dist[i][j] = distance( data[i], data[j], F );
		}
	}
	if (verbose) { fprintf(verbose, "%.0lf second(s).\n", difftime(time(NULL),t0)); }


	t0 = time(NULL);
	if (verbose && (M-T)) { fprintf(verbose, "# Compute pairwise distance for %i test instances...", M-T); }
	for (i=T; i<M; i++) {
		if (verbose && !(i%vperiod)) { fprintf(verbose, "."); }
		for (j=0; j<N; j++) {
			stored_dist[i][j] = distance( data[i], data[j], F ); 
		} 
	}
	if (verbose) { fprintf(verbose, "%.0lf second(s).\n", difftime(time(NULL),t0)); }
	
	// free original input data
	for (i=0; i<M; i++) { free(data[i]); }
	free(data);

	return stored_dist;
}

