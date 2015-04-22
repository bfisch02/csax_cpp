#include "svmlearner.hpp"
#include "svm.h"

#include "vectormath.hpp"


//RELEVANT:

/*
 struct svm_node
{
	int index;
	double value;
};

struct svm_problem
{
	int l;
	double *y;
	struct svm_node **x;
};
 */

/*
 
 * This may be relevant:
 * 
 
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
 */


/*
 * Also helpful: The old file reader, it produces an svm_problem
 
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

 */

unsigned SvmRegressor::columnCount(){
  unsigned columns = st.contvarCount - 1;
  for(unsigned i = 0; i < st.catvarCount; i++){
    columns += st.catvarSizes[i];
  }
  columns += 2 * st.binvarCount;
  return columns;
}

fracfloat_t SvmRegressor::contvarValResolveUnknowns(Sample s, unsigned index){
  return s.contvarUnknown(index) ? st.contvarMeans[index] : s.contvars[index];
}

void SvmRegressor::loadNode(svm_node* nodeInWhichToLoad, Sample s){
  //Copy each contvar except for one.
  for (unsigned i = 0; i < featureIndex; i++) {  //For each contvar
    nodeInWhichToLoad[i].index = i+1;
    nodeInWhichToLoad[i].value = contvarValResolveUnknowns(s, i);
  }
  for(unsigned i = featureIndex + 1; i < st.contvarCount; i++){
    nodeInWhichToLoad[i - 1].index = i;
    nodeInWhichToLoad[i - 1].value = contvarValResolveUnknowns(s, i);
  }
  
  unsigned loadIndex = st.contvarCount - 1;
  
  //Load categorical features
  //(Converted into a vector of size (number of categories in the feature))
  for(unsigned i = 0; i < st.catvarCount; i++){
    for(unsigned j = 0; j < st.catvarSizes[i]; j++){
      nodeInWhichToLoad[loadIndex + j].value = 0.0;
      nodeInWhichToLoad[loadIndex + j].index = loadIndex + j;
    }
    //If it wasn't unknown, put the category's feature to 1.0
    if(!s.catvarUnknown(i)){
      nodeInWhichToLoad[loadIndex + s.catvars[i]].value = 1.0;
    }
    loadIndex += st.catvarSizes[i];
  }
  
  for(unsigned i = 0; i < st.binvarCount; i++){
    if(s.binvarUnknown(i)){
      nodeInWhichToLoad[loadIndex + 0].value = 0.0;
      nodeInWhichToLoad[loadIndex + 1].value = 0.0;      
    } else if(s.binvars[i]){
      nodeInWhichToLoad[loadIndex + 0].value = 1.0;
      nodeInWhichToLoad[loadIndex + 1].value = 0.0;
    } else {
      nodeInWhichToLoad[loadIndex + 0].value = 0.0;
      nodeInWhichToLoad[loadIndex + 1].value = 1.0;
    }
    nodeInWhichToLoad[loadIndex + 0].index = loadIndex + 0;
    nodeInWhichToLoad[loadIndex + 1].index = loadIndex + 1;

    loadIndex += 2;
  }

  //Then add the sentinel value.
  nodeInWhichToLoad[loadIndex].index = -1;
  nodeInWhichToLoad[loadIndex].value = 0.0;
}

void SvmRegressor::train(Array<Sample> training){
  
  svm_problem prob;
  //initialize the svm_problem struct.
  
  //These structs are a bit of a beast.  y are the target values, and x are samples, in the form of an array terminated with index -1.  I'm not sure why the double indirection is there, and the extra value at the end is a sentinel 0.

  unsigned columns = columnCount();
  unsigned step = (columns + 1);//+1 for the terminator sentinel.

  svm_node* probx[training.length];
  double proby[training.length];
  
  //Use this blob to combat the double indirection cost and reduce malloc overead.
  //svm_node blob[step * training.length];
  
  //This blob is needed by the model during its lifetime, but we keep track of it to free it later.  C'est la vie
  blob = new svm_node[step * training.length];
  
  prob.l = training.length;
	prob.x = probx;
	prob.y = proby;
  
  
  for (unsigned i = 0; i < training.length; i++) { 
    prob.x[i] = blob + step * i; 
    loadNode(prob.x[i], training[i]);
    prob.y[i] = (double)training[i].contvars[featureIndex];
  }
  
  //dump_svm_problem(stdout, &prob, "");
  model = svm_train(&prob, params);
}

fracfloat_t SvmRegressor::predict(Sample sample){
  
  unsigned columns = columnCount();
  
  svm_node* nodes = new svm_node[columns + 1];
  
  loadNode(nodes, sample);
  
  double result = svm_predict(&model, nodes);
  
  delete [] nodes;
  
  //std::cout << "SVM prediction: " << result << std::endl;
  
  return (fracfloat_t) result;
}

