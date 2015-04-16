/*

	2010/Mar, noto@cs.tufts.edu

	This is a modified version of LOF (from Umaa) which takes TRAINING and TEST
	(see comments for constants N,C, and C0) examples, and considers each TEST
	example to NOT be part of the general population (e.g. a TEST example may
	still be an outlier if it is in the same neighborhood with other TEST
	examples), and calculates the TEST set LOF scores en masse.

	Input is comma-[and newline]-separated data.

*/

#include "oflib.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
	

// distance(i,j) stored in `stored_dist' (in main).  Only allocated stored_dist where i > j 
#define dist(i,j) ((i > j) ? stored_dist[i][j] : stored_dist[j][i])


/** Used to measure each object's neighbors (need distance for sorting) */
typedef struct {
	int index; 
	double distance;
} neighbor;

// for sorting neighbors ascending by distance
int neighborcmp(const void *a, const void *b);
	
extern const char *distance_function_name();

//LOF
int main (const int argc, const char **argv) {
	
	FILE *verbose = stderr; 	// set to NULL to turn off verbose progress messages.

	char SYNOPSIS[4000]; 
	sprintf(SYNOPSIS, "Compute LOF score for a set of instances.  Input is a set of instances, one per line, with feature values separated by commas.  This is a special implementation that allows semi-supervised anomaly detection by specifying the number of training and testing instances.  The LOF for a test example is then computed considering the distance to and among *training* instances, but not the distance to other *test* set instances (which are themselves possibly outliers).  The input file consists of all training instances, followed by all testing instances (the number of training instances plus the number of testing instances should equal the number of lines of input).  If the number of testing instances is not supplied, the program will print the LOF for all instances (i.e., unsupervised anomaly detection).  This implementation uses the distance function:  %s.", distance_function_name());

	char USAGE[2000]; 
	sprintf(USAGE, "Usage:  %s <MinPtsLB> <MinPtsUB> <No.Features> <Train.set.size> [<Test.set.size>]  <  <comma-separated-input>", argv[0]);

	const parameter_set param = parse_command_line(argc,argv,SYNOPSIS,USAGE);
	const int T = param.T; const int C0 = param.C0; const int C = param.C; 
	const int M = param.M; const int N = param.N; 
	const int MinPtsLB = param.kLB; const int MinPtsUB = param.kUB;

	const int vperiod = T/20 + 100 - (T/20)%100;	// adjust to a percentage of training set, round to nearest 100

	double **stored_dist = compute_distance(&param, verbose); // subroutine to read data, compute distance.

	// LOF 

	int i,j,p,k;
	time_t t0;

	t0 = time(NULL);
	if (verbose) { fprintf(verbose, "# Find %i neighbors for each of %i instances...", MinPtsUB, M); }

	// The efficient way to proceed is to find the MinPtsUB (upper bound)
	// nearest neighbors for each example, then choose a subset among them for
	// all k <= MinPtsUB
		
	neighbor **knn = malloc(sizeof(neighbor*) * M); // a neighbor is an index and its distance, freed at the bottom of the program
	for (i=0;i<M;i++) { 
		knn[i] = malloc(sizeof(neighbor)*MinPtsUB); 
		for (k=0; k<MinPtsUB; k++) { 
			knn[i][k].index = -1;		
			knn[i][k].distance = 1e999;
		}
	}
	
	// loop to find nearest neighbors and calculate k-distance
  	for (i=0; i<M; ++i) {
		if (verbose && !(i%vperiod)) { fprintf(verbose, "."); }
		for (j=0; j<N; ++j) {
			if (i==j) { continue; }
	  		if (dist(i,j) < knn[i][MinPtsUB-1].distance) { 
				// we found a nearer neighbor

				// overwrite most distant neighbor
				knn[i][MinPtsUB-1].index = j;
				knn[i][MinPtsUB-1].distance = dist(i,j);

				// sort so that knn[i] is a list of MinPtsUB neighbors, closest one first, 
				// index knn[i][MinPts-1] is the most distant neighbor
				qsort(knn[i], MinPtsUB, sizeof(neighbor), neighborcmp);

	  		}
		}

	} // next i
	
	if (verbose) { fprintf(verbose, "%.0lf second(s).\n# Compute LOF for %i <= k <= %i", difftime(time(NULL),t0), MinPtsLB, MinPtsUB); }

	// store the (max) LOF for C test examples
	double max_LOF[C];
	for (i=0; i < C; i++) { max_LOF[i]=0.0; }

	t0 = time(NULL);
	for (k=MinPtsLB; k<=MinPtsUB; k++) {

		if (verbose) { fprintf(verbose,"."); }
			
		double **reach_dist = malloc(sizeof(double*)*M);
		for (i=0; i<M; i++) { reach_dist[i] = malloc(sizeof(double)*k); } // freed at bottom of loop

	  	// reach-dist
	  	for (i=0; i<M; ++i) {
			for (j=0; j<k; ++j) {
		 		int nn_i = knn[i][j].index;
		  		reach_dist[i][j] = (dist(i,nn_i)) > knn[nn_i][k-1].distance ? (dist(i,nn_i)) : knn[nn_i][k-1].distance;
			}
	  	}


	  	// lrd
	  	double *lrd = malloc(sizeof(double)*M); 	// freed at the bottom of loop

	  	for (i=0; i<M; ++i) {
			double sum_reach_dist = 0.0;
			for (j=0; j<k; ++j) {
				sum_reach_dist += reach_dist[i][j];
			}
			lrd[i] = k / sum_reach_dist;

		}

		// LOF
		for (i=C0; i < C0+C; i++) { 

			//calculate LOF
			double sum_lrd = 0.0;
			for (p=0; p < k; ++p) {
		  		sum_lrd += lrd[knn[i][p].index]/lrd[i];
			}

			int test_index = i - C0;
			double LOF_k_test_index = sum_lrd / k;
			max_LOF[test_index] = (!isinf(LOF_k_test_index) && LOF_k_test_index > max_LOF[test_index]) ? LOF_k_test_index : max_LOF[test_index];	// keep max LOF
        
	  	}

		free(lrd);
		for (i=0; i<M; i++) { free(reach_dist[i]); }
		free(reach_dist);
	
	
	} // next value of k
		
	if (verbose) { fprintf(verbose, "%.0f second(s).\n", difftime(time(NULL),t0)); }
			
	for (i=0;i<M;i++) { free(knn[i]); }
	free(knn);

	// output:  print max(LOF(i)) for all test cases
	for (i=0; i<C; i++) { 
		fprintf(stdout, "%g\n", max_LOF[i]);
	}

	// free stored_dist
	for (i=0; i<M; i++) { free(stored_dist[i]); }
	free(stored_dist);

	return 0;
}

int neighborcmp(const void *a, const void *b) {
	neighbor *na = (neighbor*)a;
	neighbor *nb = (neighbor*)b;
	if (na->distance < nb->distance) { return -1; }
	if (na->distance > nb->distance) { return  1; }
	if (na->index    < nb->index)    { return -1; }
	if (na->index    > nb->index)    { return  1; }
	return 0;
}

