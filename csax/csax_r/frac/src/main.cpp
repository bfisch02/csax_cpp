/*

	FRaC.LIBSVR (frac.libsvr) 
	
	This code is mostly copied from LIBSVM's svm-train.c and svm-predict.c 
*/

const char VERSION[] = "mad.frac";
/*
	-----------   ----------  -----------------------------------------
	2011/May/11   noto        original
	2011/Oct/15   noto        0.01 (changed defaults, removed extraneous "batch" options for general use)
	2011/Nov/09   noto        mad.frac for mad 
	2011/Dec/05   noto        Added feature range options

*/



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

//#include <assert.h> // noto
#include <time.h> // noto
#include <math.h> // noto (log)
#include <sys/stat.h> // noto (file_exists)

#include "frac.h"
#include "svm.h"

#define Malloc(type,n) (type *)malloc((n)*sizeof(type))

void print_null(const char *s) {}
void exit_with_help();
void exit_with_version();
void exit_input_error(int line_num);
void parse_command_line(int argc, char **argv); //noto// char *input_file_name, char *model_file_name
svm_problem* read_problem(const char *filename);

void dump_svm_problem(FILE *out, const svm_problem *p, const char *desc);
int count_features(const svm_problem *prob, const int lower_bound); // noto, count features in a problem
void target_swap( svm_problem *prob, int feature ); // noto, replace svm_problem's target value with the feature's
void target_replace( svm_problem *prob, int feature ); // noto, reverse of above
const char* now(); // current time as string
double* predict_set( const svm_model *model, const svm_problem *prob ); // noto, make predictions given a model and svm_problem.  
double* cross_validation(const svm_problem *prob_X, const svm_parameter *svm_param); // noto, do cross validation over training svm_prob to get predictions
int file_exists(const char *filename); // noto, Boolean 

struct svm_parameter svm_param;		// set by parse_command_line, noto changed 'param' to 'svm_param' (noto, some parameters are for FRaC)

const char *PROGRAM; // noto, name of program

int main(int argc, char **argv) {
		
	const char *error_msg;	// used variously below for error messages

	PROGRAM = argv[0]; // noto, copy program name

	// parse_command_line populates the global `svm_param' object
	parse_command_line(argc, argv); 

	// noto, define three problem structueres and input files
	struct svm_problem *prob_X = NULL;		// set by read_problem (noto moved from global and changed 'prob' to 'prob_X')
	struct svm_problem *prob_V = NULL;		// noto, the validation set, set by read_problem
	struct svm_problem *prob_Q = NULL;		// noto, the test set, set by read_problem

	// noto, num features (calculated below as max. of 3 problem sets)
	unsigned int num_features = 0; 

	// noto, check errors with parameters (otherwise, error messages are a little confusing)
	error_msg = svm_check_parameter(NULL,&svm_param); 
	if(error_msg) { fprintf(stderr,"Error: %s\n", error_msg); exit(1); }

	// noto, read 3 problems instead of one
	svm_problem** problems[] = { &prob_X, &prob_V, &prob_Q }; // convenience for applying something to all problems
	const char* problem_files[] = { svm_param.X_file, svm_param.V_file, svm_param.Q_file }; // parallel input files
	for (int p=0; p<3; p++) { 

		if (problem_files[p] && problem_files[p][0]) { 
			// fprintf(stderr, "# %s, Read \"%s\" into memory...\n", now(), problem_files[p]);
			*(problems[p]) = read_problem(problem_files[p]);
		
			//debug// dump_svm_problem(stderr, *(problems[p]), problem_files[p]);

			error_msg = svm_check_parameter(*(problems[p]),&svm_param); 
			if(error_msg) { 
				fprintf(stderr,"Error (%s): %s\n",problem_files[p], error_msg);
				exit(1); 
			}
	
			num_features = count_features(*(problems[p]), num_features); // max of all problems
		}
	}

	// noto, for each (1-origin) feature 
	const unsigned int f1 = svm_param.f1;
	const unsigned int fD = svm_param.fD ? svm_param.fD : num_features;
	for (unsigned int i = f1; i <= fD; i++) {

		// noto, update progress to screen
		fprintf(stderr, "# %s, Feature %d of %d\n", now(), i, num_features);
		
		// noto, swap feature values with target
		for (int p=0; p<3; p++) { if (*(problems[p])) { target_swap( *(problems[p]), i); } }

		// noto, train model
		svm_model *model = svm_train(prob_X, &svm_param);  

		// noto, option to save/load svm_model to/from file
		//	if(svm_save_model(model_file_name, model)) { ...error ... }
		//	if((model=svm_load_model(model_file_name))==0) { ...error... }

		// noto, make predictions on validation set
		int cnt_v; // number of observations/predictions (depends on if if the validation set is given separately)
		const double *y_v; // cnt_v observations (initialized below)
		double *g_v; // cnt_v predictions (initialized below)

		if (prob_V) { 

			// Validation set was given explicitly, so we simply use the model
			// to make predictions
			cnt_v = prob_V -> l; 
			y_v = prob_V->y;
			g_v  = predict_set(model, prob_V); 

		} else { 
			
			// otherwise, use cross-validation to create a validation set from
			// training set examples and make predictions
			cnt_v = prob_X->l; 
			y_v = prob_X->y;
			g_v = cross_validation( prob_X, &svm_param );
			
		}

		// compute cnt_v errors from cnt_v predictions
		double *errors = Malloc(double, cnt_v); 
		for (int v=0; v<cnt_v; v++) { 
			errors[v] = g_v[v] - y_v[v];
		}
		free(g_v);
		
		// noto, model feature distribution (do this after predictions, since cross_validation may have set up validation sets)
		double entropy_i = entropy(y_v, cnt_v); // the entropy of feature i
				
		// noto, model the predictive error
		error_model E_i = model_error(errors, cnt_v); 
		// const double entropy_E_i = entropy( E_i );
		// double info_gain = entropy_i-entropy_E_i; // may be useful to report

		// noto, make predictions on test set
		double *g_t = predict_set( model, prob_Q );

		// noto, add normalized surprisal to each (0-origin) test instance index q
		for (int q=0; q<prob_Q->l; q++) {  

			double y = prob_Q->y[q];
			double g = g_t[q];
			double p = likelihood( g-y, E_i ); // p(x_i = g | C_i, x\x_i)
			double s = -lg(p);		// surprisal
			if (p==0 || isnan(p)) { 
				fprintf(stderr, "Warning:  probability of test instance #%d underflowed.\n", q+1);
				s = MAX_SURPRISAL; // don't let surprisal <- inf because of underflow (prob. density of Gaussian can't truly be zero)
			}
			double ns = s - entropy_i; // normalized surprisal
			if (ns > MAX_NORMALIZED_SURPRISAL) { 
				fprintf(stderr, "Warning:  normalized surprisal of test instance #%d overflowed.\n", q+1);
				ns = MAX_NORMALIZED_SURPRISAL;
			}

			fprintf(stdout, "%f%c", ns, q+1==prob_Q->l ? '\n' : '\t'); // write ns to file, if last entry in this line, write a newline
			if (q+1==prob_Q->l) { fflush(stdout); }

		}
		fflush(stdout);
		free(g_t);
		svm_free_and_destroy_model(&model);
		
		// noto, replace target features to their original place
		for (int p=0; p<3; p++) { if (*(problems[p])) { target_replace( *(problems[p]), i); } }

	} // noto, next feature

	// all done!
	svm_destroy_param(&svm_param);

	if (prob_V) { free(prob_V->y); free(prob_V->x); }
	if (prob_X) { free(prob_X->y); free(prob_X->x); }
	if (prob_Q) { free(prob_Q->y); free(prob_Q->x); }
	
	return 0;

}

int file_exists(const char *filename) {
    struct stat dummy;
	return !stat(filename, &dummy);
}

// noto, current time as string
const char* now() { 
	static char buf[26];  // 25 characters like "Fri May 13 11:06:51 2011\n\0"
	time_t t = time(NULL);
	struct tm *local = localtime(&t);
	sprintf(buf, "%s", asctime(local) );
	buf[24] = '\0'; 
	return buf;
}

// noto, make predictions given a model and svm_problem.  
// Return malloc'ed and populated array of predictions.
double* predict_set( const svm_model *model, const svm_problem *prob ) { 

	double *P = (double*)(malloc(sizeof(double) * prob->l));  // P is predictions
	for (int i=0; i<prob->l; i++) { 
		P[i] = svm_predict(model, prob->x[i]);
	}
	return P;

}

// noto, run cross-validation to make predicitions
// return dynamically allocated array of predictions 
//	parallel to prob_X->y
double* cross_validation(const svm_problem *prob_X, const svm_parameter *svm_param) {

	double *predictions = (double*) malloc(sizeof(double) * prob_X->l); 

	svm_problem TP, TU; // train prime, tune (temporary organization storage)
	TP.x = (svm_node**) malloc(sizeof(svm_node*) * prob_X->l); 
	TU.x = (svm_node**) malloc(sizeof(svm_node*) * prob_X->l); 
	TP.y = (double*) malloc(sizeof(double) * prob_X->l); 
	TU.y = (double*) malloc(sizeof(double) * prob_X->l); 

	const int folds = svm_param->folds ? svm_param->folds : prob_X->l; // if svm_param->folds is zero, use leave-one-out

	for (int fold=0; fold < folds; fold++) { 

		// divide x and y into stratified train/test sets.  I use a stratified strategy:
		// put every Nth instance in tune set, starting with fold index. 
		TP.l = TU.l = 0;
		for (int i=0; i<prob_X->l; i++) { 
			svm_problem *T = (i%(folds)==fold) ? &TU : &TP;
			T->y[T->l] = prob_X->y[i]; 
			T->x[T->l] = prob_X->x[i]; 
			T->l = T->l + 1;
		}
		
		// dump_svm_problem(stderr, &TP, "Train' set"); 
		// dump_svm_problem(stderr, &TU, "Tune set"); 

		// train and predict
		svm_model *model = svm_train(&TP, svm_param);  
		double *fold_predictions = predict_set(model, &TU); 
	
		// copy prediction back to appropriate place in output array
		int f = 0; //  index in fold_predictions
		for (int i=0; i<prob_X->l; i++) { 
			if (i%(folds)==fold) { predictions[i] = fold_predictions[f++]; }
		}

	}

	free(TP.x); free(TP.y); free(TU.x); free(TU.y);
		
	return predictions;
}

void exit_with_version() { 
	fprintf(stderr, "%s version %s\n", PROGRAM, VERSION); 
	exit(0);
}

void exit_with_help() 
{
	fprintf(stderr, 
	"\n"
	"\n"
	"THIS IS A SPECIAL VERSION OF FRaC FOR MAD SCIENCE EXPERIMENTS.\n"
	"OUTPUT IS A TABLE OF NORMALIZED SURPRISAL VALUES;\n"
	"ROWS ARE FEATURES, COLUMNS ARE TEST INSTANCES.\n"
	"\n"
	"\n"
	"This is FRaC with SVR feature models and Gaussian error models.\n"
	"All SVR code is from LIBSVM version 3.1 (See README and COPYRIGHT).\n" 
	"\n"
	"\n"
	"Usage: %s [options]\n"
	"\n"
	"  Options:\n"
	"\n"
	"    -h Print this help message and exit.\n"
	"    -v Print version information.\n"
	"\n"
	"  FRaC options:\n"
	"  \n"
	"    -X <string> Input file (required; tabular format, one data instance per line)\n"
	"    -Q <string> Test file (required)\n"
	"    -V <string> Validation file (for building error models).  If missing, do cross-validation\n"
	"    -N <integer> N-fold cross-validation for building error models (omit to do leave-one-out)\n"
	"    -T Timeout (seconds) for SVM solver optimization\n" 
	"    -1 <integer> First feature to do (for batch jobs and unfinished processes; default 1)\n"
	"    -D <integer> Last feature to do (default <number-of-features>)\n"
	"  \n" 
	"  LIBSVM options (some defaults have changed):\n"
	"  \n"
	"    -s svm_type : set type of SVM (default 0)\n"
	"    	0 -- epsilon-SVR\n"
	"    	1 -- nu-SVR\n"
	"    -t kernel_type : set type of kernel function (default 0)\n"
	"    	0 -- linear: u'*v\n"
	"    	1 -- polynomial: (gamma*u'*v + coef0)^degree\n"
	"    	2 -- radial basis function: exp(-gamma*|u-v|^2)\n"
	"    	3 -- sigmoid: tanh(gamma*u'*v + coef0)\n"
	"    	4 -- precomputed kernel (kernel values in training_set_file)\n"
	"    -d degree : set degree in kernel function (default 3)\n"
	"    -g gamma : set gamma in kernel function (default 1/num_features)\n"
	"    -r coef0 : set coef0 in kernel function (default 0)\n"
	"    -c cost : set the parameter C of C-SVC, epsilon-SVR, and nu-SVR (default 1)\n"
	"    -n nu : set the parameter nu of nu-SVC, one-class SVM, and nu-SVR (default 0.5)\n"
	"    -p epsilon : set the epsilon in loss function of epsilon-SVR (default 0)\n"
	"    -m cachesize : set cache memory size in MB (default 100)\n"
	"    -e epsilon : set tolerance of termination criterion (default 0.001)\n"
	"    -k shrinking : whether to use the shrinking heuristics, 0 or 1 (default 1)\n" // noto: changed '-h' to '-k' so '-h' could be for help
	// noto removed otions -b, -wi, -v, -q
	"\n"
	"\n"
	,PROGRAM
	);
	exit(1);
}


void exit_input_error(int line_num) {
	fprintf(stderr,"Wrong input format at line %d\n", line_num);
	exit(1);
}

void parse_command_line(int argc, char **argv) //noto// char *input_file_name, char *model_file_name
{
	int i;
	void (*print_func)(const char*) = NULL;	// default printing to std. out

	// default values
	svm_param.svm_type = EPSILON_SVR; // default for frac.libsvr is EPSILON_SVR, not C_SVC
	svm_param.kernel_type = LINEAR; // noto changed default from RBF
	svm_param.degree = 3;
	svm_param.gamma = 0;	// 1/num_features
	svm_param.coef0 = 0;
	svm_param.nu = 0.5;
	svm_param.cache_size = 100;
	svm_param.C = 1;
	svm_param.eps = 1e-3;
	svm_param.p = 0.0;	// noto changed default from 0.1
	svm_param.shrinking = 1;
	svm_param.probability = 0;
	svm_param.nr_weight = 0;
	svm_param.weight_label = NULL;
	svm_param.weight = NULL;
	svm_param.timeout = 86400; // noto

	// noto, svm_param default values
	svm_param.X_file = 
	svm_param.V_file = 
	svm_param.Q_file = NULL;
	svm_param.folds = 0; // zero will be interpreted later as leave-one-out
	svm_param.f1 = 1;
	svm_param.fD = 0; // zero will later be interpreted as number-of-features

	// parse options
	for (i=1; i<argc; i++) {
		
		if (argv[i][0] != '-') { exit_with_help(); } //noto, in this version all arguments are flag options

		// noto, check options that do not require an argument
		switch(argv[i][1]) { 

			case 'v': exit_with_version();
			case 'h': exit_with_help();
	
		}

		// noto, all other options require an argument.  advance 'i' to position of argument
		if (++i >= argc) { exit_with_help(); }

		switch(argv[i-1][1]) {

			// noto, FRaC options:
			case 'X':
				svm_param.X_file = argv[i]; 
				break;
			case 'V':
				svm_param.V_file = argv[i]; 
				break;
			case 'Q':
				svm_param.Q_file = argv[i]; 
				break;
			case 'N':
				svm_param.folds = atoi(argv[i]); 
				break;
			case 'T':
				svm_param.timeout = atoi(argv[i]); 
				break;
			case '1':
				svm_param.f1 = atoi(argv[i]);
				break;
			case 'D':
				svm_param.fD = atoi(argv[i]);
				break;


			// SVM options:

			case 's':
				svm_param.svm_type = atoi(argv[i]) + EPSILON_SVR; // EPSILON_SVR is the 0th feasible choice for this 
				if (svm_param.svm_type != EPSILON_SVR && svm_param.svm_type != NU_SVR) { fprintf(stderr, "\nIllegal option: -s %d\n\n", atoi(argv[i])); exit_with_help(); } // noto
				break;
			case 't':
				svm_param.kernel_type = atoi(argv[i]);
				break;
			case 'd':
				svm_param.degree = atoi(argv[i]);
				break;
			case 'g':
				svm_param.gamma = atof(argv[i]);
				break;
			case 'r':
				svm_param.coef0 = atof(argv[i]);
				break;
			case 'n':
				svm_param.nu = atof(argv[i]);
				break;
			case 'm':
				svm_param.cache_size = atof(argv[i]);
				break;
			case 'c':
				svm_param.C = atof(argv[i]);
				break;
			case 'e':
				svm_param.eps = atof(argv[i]);
				break;
			case 'p':
				svm_param.p = atof(argv[i]);
				break;
			case 'k':
				svm_param.shrinking = atoi(argv[i]);
				break;
			// 'q' option removed
			//case 'q':
			//	print_func = &print_null;
			//	i--; // because no argument required
			//	break;
			// 'w' weight option removed
			default:
				fprintf(stderr,"Unknown option: -%c\n", argv[i-1][1]);
				exit_with_help();
		}
	}

	// noto, check for "required options"
	int error=0;
	if (!svm_param.X_file) { fprintf(stderr, "Missing required argument: -X\n"); error++; }
	if (!svm_param.Q_file) { fprintf(stderr, "Missing required argument: -Q\n"); error++; }
	if (error) { exit_with_help(); }

	svm_set_print_string_function(print_func);

} // parse_command_line


// lookahead and count columns in the current row of the input (file), then
// reset the file position.
int count_columns(FILE *fin) {

	int columns = -1; // return value 

	long start = ftell(fin);

	int white = 1;
	char c;
	for (c=fgetc(fin); c >= 0; c=fgetc(fin)) { 
		if (c=='\n') { 
			columns++;  // 'columns' is always one less than it needs to be, since we want -1 to be a special indicator of no fin at all.
			break;
		} else if (c > ' ' && white) { 
			columns++;
			white = 0;
		} else if (c <= ' ') { 
			white = 1;
		}
	}
	
	fseek(fin, start, SEEK_SET); 

	return columns;

}

// lookahead and count rows in file stream from the current point and reset the
// file pointer
int count_rows(FILE *fin) { 

	char c;
	int rows = 0;
	int last_char_newline = 1;

	long start = ftell(fin);

	for (c=fgetc(fin); c>=0; c=fgetc(fin)) {
		if (c=='\n') { rows++; last_char_newline = 1; }
		else { last_char_newline = 0; }
	}

	if (!last_char_newline) { rows++; } // add one for line that doesn't end with newline

	fseek(fin, start, SEEK_SET);

	return rows;

}

// skip white space
int ws(FILE *fin) { 
	char c;
	for (c=fgetc(fin); 0 <= c && c <= ' '; c=fgetc(fin));
	return ungetc(c, fin);
}

// read problem in tabular format (rows are instances, columns are features)
// return pointer to dynamically allocated problem
svm_problem* read_problem(const char *filename) { 

	svm_problem *prob = (svm_problem*) malloc(sizeof(svm_problem));

	FILE *fin = fopen(filename,"r");
	if (!fin) { fprintf(stderr, "\nError reading \"%s\"\n\n", filename); exit(-1);  }

	const int n = count_rows(fin);
	prob->x = (svm_node**) malloc(n * sizeof(svm_node*));
	prob->l = 0;

	const int d = count_columns(fin);
	fprintf(stderr, "# \"%s\" has %d row(s) and %d column(s)\n", filename, n, d); 
	
	int i,lineno;
	for (lineno=1; lineno<=n; lineno++) { 
		int columns = count_columns(fin); 

		if (columns < 0 ) { 
			break;  // EOF
		} else if (columns == 0) { 
			fprintf(stderr, "Line %d of fin has no data.\n", lineno);
			prob->l--;
		} else if (columns != d) { 
			fprintf(stderr, "Line %d of fin has %d != %d columns of data.\n", lineno, columns, d); 
			exit(-1);
		} else { 
			prob->x[prob->l] = (svm_node*) malloc( (columns+1) * sizeof(svm_node) );
			for (i=0; i<columns; i++) { 
				prob->x[prob->l][i].index = i+1;
				fscanf(fin, "%lg", &(prob->x[prob->l][i].value));
				ws(fin); // skip white space
			}
			prob->x[prob->l][columns].index = -1;
			prob->x[prob->l][columns].value = 0.0;
			prob->l++;
		}
	}	
	prob->y = (double*) malloc(prob->l * sizeof(double));  // don't bother initializing values of y

	fclose(fin);

	return prob;

} // read_problem




// noto, count features in a problem
int count_features(const svm_problem *prob, const int lower_bound) {

	int d = lower_bound; // return value, d = number of features

	for (int i=0; i<prob->l; i++) { 
		for (svm_node *p = prob->x[i]; p->index > 0; p++) { 
			d = p->index > d ? p->index : d;
		}
	}

	return d;
}

void target_swap( svm_problem *prob, int feature ) {
	for (int i=0; i<prob->l; i++) { 
		prob->y[i] = 0;
		for (svm_node *x = prob->x[i]; x->index > 0; x++) { 
			if (x->index == feature) { 
				prob->y[i] = x->value;
				x->value = 0;
				break;
			}
		}
	}
}

void target_replace( svm_problem *prob, int feature ) {
	for (int i=0; i<prob->l; i++) { 
		for (svm_node *x = prob->x[i]; x->index > 0; x++) { 
			if (x->index == feature) { 
				x->value = prob->y[i];
				break;
			}
		}
		prob->y[i] = 0;
	}
}

// noto, dump svm_problem
void dump_svm_problem(FILE *out, const svm_problem *p, const char *desc) {

	fprintf(out, "------ SVM PROBLEM %s ----------\n", desc);
	fprintf(out, "\n");
	fprintf(out, "l = %d\n", p->l); 
	fprintf(out, "\n");
	
	fprintf(out, "y = ["); 
	for (int i=0; i<p->l; i++) { fprintf(out, "%g,", p->y[i]); }
	fprintf(out, "]\n");

	fprintf(out, "\n");


	for (int i=0; i<p->l; i++) { 
		
		fprintf(out, "x[%d] = [", i);

		int prev_index = -1;
		svm_node *x;
		for (x = p->x[i]; x->index > prev_index; x++) { 
			fprintf(out, " %d:%g", x->index, x->value); 
			prev_index = x->index;
		}
		fprintf(out, "] (%d <= %d, %g)\n", x->index, prev_index, x->value);

	}
	fprintf(out, "\n\n\n");

}
