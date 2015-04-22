/*

	FRaC.LIBSVR (frac.libsvr) 
	
	This code is mostly copied from LIBSVM's svm-train.c and svm-predict.c 
*/

const char* VERSION = "0.02";
/*
	-----------   ----------  -----------------------------------------
	2011/May/11   noto        original
	2011/Oct/15   noto        0.01 (changed defaults, removed extraneous "batch" options for general use)
  2015/Dec/25   cousins     categorical feature support, arff format, modular design
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include <assert.h>
#include <time.h>
#include <math.h>

#include <parallel/settings.h> 

#include "frac.hpp"
#include "svm.h"
#include "io.hpp"
#include "errormodel.hpp"
#include "svmlearner.hpp"
#include "waffles.hpp"
#include "io.hpp"

#include "vectormath.hpp"
#include "evaluation.hpp"

#include "sampletransformer.hpp"

//Function Declarations:

static void exit_with_help();
static void exit_with_version();
static void parse_command_line(int argc, char **argv);

const char *PROGRAM; //Binary name

///////////
//Globals//
///////////

///////
//Input

#define TRAINSET 0
#define VALISET  1
#define QUERYSET 2
#define OUTFILE  3

#define FILEPATHCOUNT 4
char *ioFilePaths[FILEPATHCOUNT] = {0};

//Input format can be either ARFF, for standard use, or FRaC, for training.
enum InputFormat {
  input_ARFF, input_FRaC_test
};
InputFormat inputFormat = input_ARFF;

//In ARFF mode, FRaC can be run with a validation set, with cross validation, or in mode_unsupervised mode.
#define InputDataUsage_LENGTH 3
enum InputDataUsage {
  mode_withValidationSet, mode_crossValidation, mode_unsupervised
};
InputDataUsage inputDataUsage = mode_crossValidation;

////////
//Output

//OUTPUT OPTIONS:
enum OutMode { arff_with_anom_score_sorted, arff_with_anom_score_unsorted, anom_score };
OutMode outMode = arff_with_anom_score_sorted;

////////////
//Parameters

//Default svm
svm_parameter svm_param = {EPSILON_SVR, LINEAR, 3, 0, 0, 100, 1e-3, 1, 0, NULL, NULL, 0.5, 0.0, 1, 0, 86400};

#define SVM_ENSEMBLE_SIZE 4
svm_parameter svm_param_ensemble[SVM_ENSEMBLE_SIZE] = {
  {EPSILON_SVR, LINEAR, 3, 0, 0, 100, 1e-3, 1, 0, NULL, NULL, 0.5, 0.0, 1, 0, 86400},
  //{EPSILON_SVR, POLY, 3, 0, 0, 100, 1e-3, 1, 0, NULL, NULL, 0.5, 0.0, 1, 0, 86400},
  //{EPSILON_SVR, RBF, 3, 0, 0, 100, 1e-3, 1, 0, NULL, NULL, 0.5, 0.0, 1, 0, 86400},
  {EPSILON_SVR, SIGMOID, 3, 0, 0, 100, 1e-3, 1, 0, NULL, NULL, 0.5, 0.0, 1, 0, 86400},
  {NU_SVR, LINEAR, 3, 0, 0, 100, 1e-3, 1, 0, NULL, NULL, 0.5, 0.0, 1, 0, 86400},
  //{NU_SVR, POLY, 3, 0, 0, 100, 1e-3, 1, 0, NULL, NULL, 0.5, 0.0, 1, 0, 86400},
  //{NU_SVR, RBF, 3, 0, 0, 100, 1e-3, 1, 0, NULL, NULL, 0.5, 0.0, 1, 0, 86400},
  {NU_SVR, SIGMOID, 3, 0, 0, 100, 1e-3, 1, 0, NULL, NULL, 0.5, 0.0, 1, 0, 86400}
};

//CROSS VALIDATION OPTIONS:
unsigned int cvFolds = 0; // 0 -> LOOCV
unsigned int cvShuffles = 0; // How many shuffles to perform (useful for cv other than LOOCV).

////////////////////////
//FRaC algorithm options

//Division FRaC options:
bool useDivisionFRaC, useNaiveDivisionFRaC;

//Unknown Value Correction FRaC
bool useUnknownCorrection = false;

//Heuristic Options
#define FilterMode_LENGTH 4
enum FilterMode { filter_random, filter_entropy, filter_entropy_transductive, filter_klDivergence };
FilterMode filterMode = filter_random;
fracfloat_t filterCutoff = 0;
bool usepfFRaC = false;


#define EnsembleType_LENGTH 5
enum EnsembleType { ensemble_none, ensemble_inner, ensemble_outer, ensemble_inner_small, ensemble_outer_small };
EnsembleType ensembleType = ensemble_none;

//WAFFLES OPTIONS:
unsigned randomForestSize = 32;
unsigned decisionTreeBucketMin = 0;

//Error Model Options
#define ContErrorModelType_LENGTH 4
enum ContErrorModelType { em_gaussian, em_nbin_multigaussian, em_radn_multigaussian, em_radn_multigaussian_weighted };
ContErrorModelType contErrorModelType = em_radn_multigaussian;

//Output control
bool verbose = false;

//Term score logging

bool logTermScores = false;
char* termScoreLogFile = 0;

static unsigned readUnsignedSafe(char* s, unsigned max, char* optname){
  
  char *end;
  unsigned long l = strtol(s, &end, 0);
  #ifndef UNSAFE
  if (*s == '\0' || *end != '\0') {
    std::cerr << "ERROR: " << "Option " << optname << " must be followed by an integer.  Found \"" << s << "\"." << std::endl;
    exit(1);
  }
  
  if(l > max){
    std::cerr << "ERROR: " << "Expected value in [0, " << max << "]" << ", found " << l << ", for " << optname << std::endl;
    exit(1);
  }
  #endif

  return (unsigned)l; //Cast is safe in SAFE mode, because we already know it's less than max.
}

static unsigned readUnsignedSafe(char* s, char* optname){
  
  char *end;
  unsigned long l = strtol(s, &end, 0); // Unfortunately, strtou doesn't exist.
  #ifndef UNSAFE
  if (*s == '\0' || *end != '\0') {
    std::cerr << "ERROR: " << "Option " << optname << " must be followed by an integer.  Found \"" << s << "\"." << std::endl;
    exit(1);
  }
  
  //Check for overflow.
  if(l > std::numeric_limits<unsigned>::max() ){
    std::cerr << "ERROR: " << "Overflow Error.  Option " << optname << " must be followed by an unsigned integer, max is " << std::numeric_limits<unsigned>::max() << ".  Found \"" << l << "\"." << std::endl;
    exit(1);
  }
  #endif

  return (unsigned)l;
}

#define readEnumSafe(ENUM, len, s, optname) ((ENUM) readUnsignedSafe(s, (len - 1), optname))

//Safely read in a float
static float readFloatSafe(char* s, char* optname, float min, float max){
  
  char *end;
  float f = strtof(s, &end); // Unfortunately, strtou doesn't exist.
  #ifndef UNSAFE
  if (*s == '\0' || *end != '\0') {
    std::cerr << "ERROR: " << "Option " << optname << " must be followed by a float.  Found \"" << s << "\"." << std::endl;
    exit(1);
  }
  
  //Check for overflow.
  if(f > std::numeric_limits<unsigned>::max() ){
    std::cerr << "ERROR: " << "Expected value in [" << min << ", " << max << "]" << ", found" << f << ", for " << optname << "." << std::endl;
    exit(1);
  }
  #endif

  return f;
}

//Main function, runs FRaC
int main(int argc, char **argv)
{
  //Paralellism Configuration
  
  __gnu_parallel::_Settings s;
  s.for_each_minimal_n = 1;
  s.algorithm_strategy = __gnu_parallel::force_parallel;
  __gnu_parallel::_Settings::set(s);
  
  
	PROGRAM = argv[0]; // noto, copy program name

	// parse_command_line sets svm_param
	parse_command_line(argc, argv);
  
  if(verbose){
    std::cerr << "FRaC initializing." << std::endl;
  }

  SampleStructure sampleStructure;
  
  //Load the input files.
  Array<Sample> training, validation, query;
  
  //Set all lengths to 0 so that unused sets can be iterated over safely.
  training.length = validation.length = query.length = 0;
  
  Array<bool> testClasses; //Used for evaluation.
  
  switch(inputFormat){
    case input_ARFF:
    {
      //Parallel: Could load these in parallel
      //Load the training set.
      StructuredSampleCollection tsss = readArff(ioFilePaths[TRAINSET]);
      #ifndef UNSAFE
      if(tsss.samples.length == 0){
        std::cerr << "Warning: Empty training set." << std::endl;
        exit(1);
      }
      #endif
      sampleStructure = tsss.sampleStructure;
      training = tsss.samples;
      
      if(inputDataUsage == mode_withValidationSet){
        StructuredSampleCollection vsss = readArff(ioFilePaths[VALISET]);
        #ifndef UNSAFE
        if(vsss.samples.length == 0){
          std::cerr << "Error: Empty validation set." << std::endl;
          exit(1);
        }
        
        if(vsss.sampleStructure != sampleStructure){
          std::cerr << "Error: Training and validation sets contain incompatible data." << std::endl;
          std::cerr << "Training:   " << tsss.sampleStructure << std::endl;
          std::cerr << "Validation: " << vsss.sampleStructure << std::endl;
          exit(1);
        }
        #endif
        validation = vsss.samples;
        vsss.sampleStructure.freeData();
      }
      if(inputDataUsage != mode_unsupervised){
        StructuredSampleCollection qsss = readArff(ioFilePaths[QUERYSET]);
        #ifndef UNSAFE
        if(qsss.samples.length == 0){
          std::cerr << "Error: Empty query set." << std::endl;
        }
        
        if(qsss.sampleStructure != sampleStructure){
          std::cerr << "Error: Training and query sets contain incompatible data." << std::endl;
          std::cerr << "Training: " << tsss.sampleStructure << std::endl;
          std::cerr << "Query:    " << qsss.sampleStructure << std::endl;
          exit(1);
        }
        #endif
        query = qsss.samples;
        qsss.sampleStructure.freeData();
      }

      break;
    }
    case input_FRaC_test:
    {
      TestFramework evaluation = loadTestData(std::string(ioFilePaths[TRAINSET]));

      sampleStructure = evaluation.ss;
      
      training = evaluation.training;
      query = evaluation.test;
      
      testClasses = evaluation.testClasses;
      
      break;
    }
    default:
      assert(0);
  }

  #ifdef UNKNOWNVALUES
  sampleStructure.contvarMeans = StructuredSampleCollection(sampleStructure, training).calculateMeanContFeatures();
  #else
  sampleStructure.contvarMeans = NULL;
  #endif
  
  //Verbose Output
  if(verbose){
    std::cerr << "Read dataset with " << sampleStructure.featureCount() << " features (" << sampleStructure.contvarCount << " continuous, " << sampleStructure.catvarCount << " categorical, and " << sampleStructure.binvarCount << " binary)." << std::endl;
    std::cerr << "Training Set Size: " << training.length << std::endl;
    if(inputDataUsage == mode_withValidationSet){
      std::cerr << "Validation Set Size: " << validation.length << std::endl;
    }
    std::cerr << "Query Set Size: " << query.length << std::endl;
  }
  
  ////////////////////
  //FRaC Preprocessing
  
  //Scale convars to prevent negative surprisal.
  if((useDivisionFRaC && (!useNaiveDivisionFRaC)) || (filterMode == filter_entropy || filterMode == filter_entropy_transductive)){
    //Optimization:  All this is recalculated in the error model collection when the entropies are calculated.  This could probably be moved there.
    //Optimization: This is parallelizable
    
    fracfloat_t scaleFactors[sampleStructure.contvarCount];
    fracfloat_t offsets[sampleStructure.contvarCount]; //Optimization: This exists so the z score normalizer can be used.  A scaler transformer would be faster.
    for(unsigned i = 0; i < sampleStructure.contvarCount; i++){
      fracfloat_t contvars[training.length];
      training.mapTo<fracfloat_t, unsigned>([](Sample s, unsigned fIndex){return s.contvars[fIndex];}, i, Array<fracfloat_t>(contvars, training.length));
    
      //Build  distribution

      //Optimization: Lots of news and deletes are going on here.
      //Detail: the choice of error model.
      MultiGaussianWeighted dist = MultiGaussianWeighted::fitGaussianKernel(contvars, training.length, (unsigned)sqrt(training.length));
      fracfloat_t max = dist.maxValue();
      dist.freeMemory();
      
      scaleFactors[i] = max;
      offsets[i] = 0; 
    }
    
    //This will stretch the distribution so the max is 1
    ZScoreAntinormalizer normalizer(sampleStructure, offsets, scaleFactors);
    
    for(unsigned i = 0; i < training.length; i++){
      normalizer.transformSampleInPlace(training[i]);
    }
    for(unsigned i = 0; i < validation.length; i++){
      normalizer.transformSampleInPlace(validation[i]);
    }
    for(unsigned i = 0; i < query.length; i++){
      normalizer.transformSampleInPlace(query[i]);
    }
    
    
    //Do not free the normalizer (it is using stack memory).
    
  }
  
  
  if(filterCutoff > 0 && !usepfFRaC){
    
    if(usepfFRaC) std::cerr << "Calculating partial filter features." << std::endl;
    SampleTransformer* transformer;
    
    switch(filterMode){
      case filter_random:
        transformer = randomFilter(sampleStructure, filterCutoff, verbose);
        break;
      case filter_entropy:
        transformer = lowEntropyFilter(sampleStructure, filterCutoff, training, verbose);
        break;
      case filter_entropy_transductive:
        transformer = lowEntropyFilterTransductive(sampleStructure, filterCutoff, training, query, verbose);
        break;
      case filter_klDivergence:
        transformer = klDivergenceFilter(sampleStructure, filterCutoff, training, query, verbose);
        break;
      default:
        assert(0);
    }
    
    //std::cerr << "Pre SS: " << sampleStructure << std::endl;
    //Parallel: Could do these 3 simultaneously.
    transformer->transformSampleStructureInPlace(sampleStructure);
    switch(inputDataUsage){
      case mode_withValidationSet:
        validation.forEach<SampleTransformer*>([](Sample& s, SampleTransformer* t){return t->transformSampleInPlace(s);}, transformer);
      case mode_crossValidation:
        training  .forEach<SampleTransformer*>([](Sample& s, SampleTransformer* t){return t->transformSampleInPlace(s);}, transformer);
      case mode_unsupervised:
        query     .forEach<SampleTransformer*>([](Sample& s, SampleTransformer* t){return t->transformSampleInPlace(s);}, transformer);
        break;
      default:
        assert(0);
    }
    //std::cerr << "Post SS: " << sampleStructure << std::endl;
    
    transformer->freeMemory();
    delete transformer;
  }
  
  
  ///////////
  //Run FRaC
  
  //Build a FRaC object.
  
  //Build a predictor (containing the ML objects (regressors, classifiers, binaryclassifiers) for each feature).
  
  //Make creator lambdas
  MlCreators mlc;

  switch (ensembleType) {
    case ensemble_none:
      
      mlc.regressorCount = mlc.classifierCount = 1;

      mlc.rc = new RegressorCreator[1];
      mlc.rc[0] =  [](SampleStructure* st, Array<Sample> training, unsigned index) {
        Regressor* r = new SvmRegressor(*st, index, &svm_param);
        r->train(training);

        return r;
      };

      mlc.cc = new ClassifierCreator[1];
      mlc.cc[0] = [](SampleStructure* st, Array<Sample> training, unsigned index) {
        Classifier* c = new WafflesDecisionTreeClassifier(*st, index);
        ((WafflesDecisionTreeClassifier*)c)->model.setLeafThresh(decisionTreeBucketMin);
        c->train(training);

        return c;
      };
      break;
      
    case ensemble_inner:
      
      mlc.regressorCount = 1;
      mlc.rc = new RegressorCreator[mlc.regressorCount];
      mlc.rc[0] = [](SampleStructure* st, Array<Sample> training, unsigned index) {
        
        Regressor** regressors = new Regressor*[SVM_ENSEMBLE_SIZE + 1]; // 4 SVM and a decision tree.
        regressors[0] = new SvmRegressor(*st, index, &(svm_param_ensemble[0]));
        regressors[1] = new SvmRegressor(*st, index, &(svm_param_ensemble[1]));
        regressors[2] = new SvmRegressor(*st, index, &(svm_param_ensemble[2]));
        regressors[3] = new SvmRegressor(*st, index, &(svm_param_ensemble[3]));
        regressors[4] = new WafflesDecisionTreeRegressor(*st, index);
        
        Regressor* r = new RegressorEnsemble(Array<Regressor*>(regressors, 5));

        r->train(training);
        return r;
      };
      
      
      mlc.classifierCount = 1;
      mlc.cc = new ClassifierCreator[mlc.classifierCount];
      mlc.cc[0] = [](SampleStructure* st, Array<Sample> training, unsigned index) {

        Classifier** classifiers = new Classifier*[6];

        classifiers[0] = new WafflesDecisionTreeClassifier(*st, index);
        classifiers[1] = new WafflesGenericClassifier(*st, index, MeanMarginsTree);
        classifiers[2] = WafflesGenericClassifier::makeRandomForest(*st, index, randomForestSize);
        classifiers[3] = new WafflesGenericClassifier(*st, index, NaiveBayes);
        classifiers[4] = new WafflesGenericClassifier(*st, index, NN2);
        classifiers[5] = new WafflesGenericClassifier(*st, index, NN3);

        Classifier* c = new ClassifierEnsemble(Array<Classifier*>(classifiers, 6), *st, index);

        c->train(training);

        return c;
      };

      break;
      
    case ensemble_outer:
      
      mlc.regressorCount = SVM_ENSEMBLE_SIZE + 1; // 4 SVM and a decision tree.
      mlc.classifierCount = 6;

      mlc.rc = new RegressorCreator[mlc.regressorCount];
      mlc.rc[0] = [](SampleStructure* st, Array<Sample> training, unsigned index) {
        Regressor* r = new SvmRegressor(*st, index, &(svm_param_ensemble[0]));
        r->train(training);
        return r;
      };
      mlc.rc[1] = [](SampleStructure* st, Array<Sample> training, unsigned index) {
        Regressor* r = new SvmRegressor(*st, index, &(svm_param_ensemble[1]));
        r->train(training);
        return r;
      };
      mlc.rc[2] = [](SampleStructure* st, Array<Sample> training, unsigned index) {
        Regressor* r = new SvmRegressor(*st, index, &(svm_param_ensemble[2]));
        r->train(training);
        return r;
      };
      mlc.rc[3] = [](SampleStructure* st, Array<Sample> training, unsigned index) {
        Regressor* r = new SvmRegressor(*st, index, &(svm_param_ensemble[3]));
        r->train(training);
        return r;
      };

      mlc.rc[4] = [](SampleStructure* st, Array<Sample> training, unsigned index) {
        Regressor* r = new WafflesDecisionTreeRegressor(*st, index);
        r->train(training);
        return r;
      };
      
      
      mlc.cc = new ClassifierCreator[mlc.classifierCount];
      mlc.cc[0] = [](SampleStructure* st, Array<Sample> training, unsigned index) {
        Classifier* c = new WafflesDecisionTreeClassifier(*st, index);
        c->train(training);
        return c;
      };
      mlc.cc[1] = [](SampleStructure* st, Array<Sample> training, unsigned index) {
        Classifier* c = new WafflesGenericClassifier(*st, index, MeanMarginsTree);
        c->train(training);
        return c;
      };
      mlc.cc[2] = [](SampleStructure* st, Array<Sample> training, unsigned index) {
        Classifier* c = WafflesGenericClassifier::makeRandomForest(*st, index, randomForestSize);
        c->train(training);
        return c;
      };
      mlc.cc[3] = [](SampleStructure* st, Array<Sample> training, unsigned index) {
        Classifier* c = new WafflesGenericClassifier(*st, index, NaiveBayes);
        c->train(training);
        return c;
      };
      mlc.cc[4] = [](SampleStructure* st, Array<Sample> training, unsigned index) {
        Classifier* c = new WafflesGenericClassifier(*st, index, NN2);
        c->train(training);
        return c;
      };
      mlc.cc[5] = [](SampleStructure* st, Array<Sample> training, unsigned index) {
        Classifier* c = new WafflesGenericClassifier(*st, index, NN3);
        c->train(training);
        return c;
      };

      break;


    case ensemble_inner_small:
      
      mlc.regressorCount = 1;
      mlc.rc = new RegressorCreator[mlc.regressorCount];
      mlc.rc[0] = [](SampleStructure* st, Array<Sample> training, unsigned index) {
        
        unsigned innerCount = 2;
        
        Regressor** regressors = new Regressor*[innerCount]; // SVM and a decision tree.
        regressors[0] = new SvmRegressor(*st, index, &(svm_param_ensemble[0]));
        regressors[1] = new WafflesDecisionTreeRegressor(*st, index);
        
        Regressor* r = new RegressorEnsemble(Array<Regressor*>(regressors, innerCount));

        r->train(training);
        return r;
      };
      
      
      mlc.classifierCount = 1;
      mlc.cc = new ClassifierCreator[mlc.classifierCount];
      mlc.cc[0] = [](SampleStructure* st, Array<Sample> training, unsigned index) {

        unsigned innerCount = 2;
        
        Classifier** classifiers = new Classifier*[innerCount];

        classifiers[0] = new WafflesDecisionTreeClassifier(*st, index); //decision tree + NN
        classifiers[1] = new WafflesGenericClassifier(*st, index, NN3);

        Classifier* c = new ClassifierEnsemble(Array<Classifier*>(classifiers, innerCount), *st, index);

        c->train(training);

        return c;
      };

      break;
      
    case ensemble_outer_small:
      
      mlc.regressorCount = 2; // SVM and a decision tree.
      mlc.classifierCount = 2;

      mlc.rc = new RegressorCreator[mlc.regressorCount];
      mlc.rc[0] = [](SampleStructure* st, Array<Sample> training, unsigned index) {
        Regressor* r = new SvmRegressor(*st, index, &(svm_param_ensemble[0]));
        r->train(training);
        return r;
      };
      mlc.rc[1] = [](SampleStructure* st, Array<Sample> training, unsigned index) {
        Regressor* r = new WafflesDecisionTreeRegressor(*st, index);
        r->train(training);
        return r;
      };
      
      
      mlc.cc = new ClassifierCreator[mlc.classifierCount];
      mlc.cc[0] = [](SampleStructure* st, Array<Sample> training, unsigned index) {
        Classifier* c = new WafflesDecisionTreeClassifier(*st, index);
        c->train(training);
        return c;
      };
      mlc.cc[1] = [](SampleStructure* st, Array<Sample> training, unsigned index) {
        Classifier* c = new WafflesGenericClassifier(*st, index, NN3);
        c->train(training);
        return c;
      };

      break;
  }
  
  mlc.binaryClassifierCount = 1;
  
  mlc.bcc = new BinaryClassifierCreator[1];
  mlc.bcc[0] = [](SampleStructure* st, Array<Sample> training, unsigned index) {
    BinaryClassifier* bc = new WafflesDecisionTreeBinaryClassifier(*st, index);
    bc->train(training);

    return bc;
  };

  //Now make error models:

  //Error model creator lambdas.

  ErrorModelCreators emc;
  
  switch (contErrorModelType){
    case em_gaussian:
      emc.rc = [](SampleStructure* st, unsigned index, contvar_t* trueData, contvar_t* predictedData, unsigned length) {
          return (ContinuousErrorModel*) new Gaussian(trueData, predictedData, length); //I don't really understand why this cast is necessary, seems like a violation of Liskov's Principle.  I believe that C++ does not overload the parameters of function pointer types?
        };
      break;
    case em_nbin_multigaussian:
      emc.rc = [](SampleStructure* st, unsigned index, contvar_t* trueData, contvar_t* predictedData, unsigned length) {
          MultiGaussian* mg = new MultiGaussian();
          *mg = MultiGaussian::fitGaussianKernelNBin(trueData, predictedData, length);
          return (ContinuousErrorModel*) mg;
        };
      break;
      
    case em_radn_multigaussian:
      emc.rc = [](SampleStructure* st, unsigned index, contvar_t* trueData, contvar_t* predictedData, unsigned length) {
          MultiGaussian* mg = new MultiGaussian();
          *mg = MultiGaussian::fitGaussianKernel(trueData, predictedData, length, (unsigned)sqrt(length));
          return (ContinuousErrorModel*) mg;
        };
      break;
      
    case em_radn_multigaussian_weighted:
      emc.rc = [](SampleStructure* st, unsigned index, contvar_t* trueData, contvar_t* predictedData, unsigned length) {
          MultiGaussianWeighted* mg = new MultiGaussianWeighted();
          *mg = MultiGaussianWeighted::fitGaussianKernel(trueData, predictedData, length, (unsigned)sqrt(length));
          return (ContinuousErrorModel*) mg;
        };
      break;
      
    default:
      assert(0);
  }
  
  emc.cc = [](SampleStructure* st, unsigned index, catvar_t* trueData, catvar_t* predictedData, unsigned length) {
    return (CategoricalErrorModel*) new SurprisalMatrix(st->catvarSizes[index], trueData, predictedData, length);
  };
  emc.bcc = [](SampleStructure* st, unsigned index, binvar_t* trueData, binvar_t* predictedData, unsigned length) {
    Array<catvar_t> trueConv = Array<binvar_t>(trueData, length).map<catvar_t>([](binvar_t bv) {
      return (catvar_t) bv;
    });
    Array<catvar_t> predConv = Array<binvar_t>(predictedData, length).map<catvar_t>([](binvar_t bv) {
      return (catvar_t) bv;
    });

    SurprisalMatrix catmodel = SurprisalMatrix(2, trueConv.data, predConv.data, length);

    trueConv.freeMemory();
    predConv.freeMemory();

    return (BinaryErrorModel*) new BinaryErrorModelFromCategoricalErrorModel<SurprisalMatrix>(SurprisalMatrix());
  };

  //Make sure shuffles and cvFolds make sense together
  if(inputDataUsage != mode_withValidationSet){
    if(cvFolds == 0 || cvFolds > training.length) cvFolds = training.length;
    if(cvFolds == training.length){ //If LOOCV
      if(cvShuffles > 0){
        std::cerr << "Warning: Attempt to use " << cvShuffles << " shuffles with LOOCV.  Defaulting to 0." << std::endl;
        cvShuffles = 0;
      }
    }
  }
  
  //Predictor and error models get created in the switch below.
  
  Predictor p;
  ErrorModelCollection e;
  
  switch(inputDataUsage){
    case mode_withValidationSet:
      //Missing values are not allowed in the training or validation sets.
      
      #ifndef UNSAFE
      //Check this property
      
      if(StructuredSampleCollection(sampleStructure, training).hasUnknowns()){
        std::cerr << "Warning: Unknown values detected in training set.  Samples with unknown values have been removed.\n";
        training.filterInPlace<SampleStructure>([](Sample s, SampleStructure ss){return s.hasUnknowns(ss);}, sampleStructure);
      }
      if(StructuredSampleCollection(sampleStructure, validation).hasUnknowns()){
        std::cerr << "Warning: Unknown values detected in training set.  Samples with unknown values have been removed.\n";
        validation.filterInPlace<SampleStructure>([](Sample s, SampleStructure ss){return s.hasUnknowns(ss);}, sampleStructure);
      }

      #endif
      
      //Build predictor from the training set.
      p = Predictor(sampleStructure, training, mlc);
      //Build error model from the validation set.
      e = ErrorModelCollection(sampleStructure, emc, training, validation, p);
      break;
      
    case mode_crossValidation:
    {
      //Missing values are not allowed in the training set.
      
      #ifndef UNSAFE
      //Check this property
      
      if(StructuredSampleCollection(sampleStructure, training).hasUnknowns()){
        std::cerr << "Warning: Unknown values detected in training set.  Samples with unknown values have been removed.\n";
        training.filterInPlace<SampleStructure>([](Sample s, SampleStructure ss){return s.hasUnknowns(ss);}, sampleStructure);
      }
      #endif
      
      if(verbose){
        std::cerr << "Training Predictor." << std::endl;
      }
      
      //Duplicate the training set
      StructuredSampleCollection trainingCopy = StructuredSampleCollection(sampleStructure, arrayCopy<Sample>(training));
      
      if(verbose){
        std::cerr << "Cross Validating Error Models and Training Predictor." << std::endl;
      }
      
      //Build predictor from the training set.
      std::thread t([&](){p = Predictor(sampleStructure, training, mlc);});
      
      //Build error model from CV on the training set.
      e = ErrorModelCollection(mlc, emc, trainingCopy, cvFolds, cvShuffles);

      if(verbose){
        std::cerr << "Cross Validation Complete." << std::endl;
      }
      
      t.join();
      
      if(verbose){
        std::cerr << "Predictor Complete." << std::endl;
      }
      
      trainingCopy.samples.freeMemory(); //Free the copy.
      break;
    }
    case mode_unsupervised:
      query = training;
      
      //Now we have to copy training data if shuffles are being used.
      
      //Filter out the unknown values.
      training = training.filter<SampleStructure>([](Sample s, SampleStructure ss){return s.hasUnknowns(ss);}, sampleStructure);
      //training = arrayCopy<Sample>(training);
      
      
      //Build predictor from the training set.
      p = Predictor(sampleStructure, training, mlc);
      //Build error model from CV on the training set.
      e = ErrorModelCollection(mlc, emc, StructuredSampleCollection(sampleStructure, training), cvFolds, cvShuffles);
      
      //Free the copied memory
      training.freeMemory();
  }
  
  //Now build the FRaC object
  
  //(SampleType st, Predictor p, ErrorModelCollection e) : st(st), predictor(p), errorModels(e) { }
  FRaC* frac;
  
  //The old versions might be an iota quicker, but they're not currently in use, in favor of the _g versions.
  /*
  if(useDivisionFRaC){
    frac = new DivisionFRaC(sampleStructure, p, e, useUnknownCorrection, usePreSubtraction);
  }
  else{
    frac = new TraditionalFRaC(sampleStructure, p, e, useUnknownCorrection);
  }
   */
  
  if(useDivisionFRaC){
    frac = new DivisionFRaC_G(sampleStructure, p, e, useUnknownCorrection);
  }
  else if(usepfFRaC){
    BitArray filter(sampleStructure.featureCount());
    
    switch(filterMode){;
      case filter_random:
      {
        unsigned possibilities[sampleStructure.featureCount()];
        for(unsigned i = 0; i < sampleStructure.featureCount(); i++){
          possibilities[i] = i;
        }


        std::random_device rd;
        std::default_random_engine g(rd());

        std::shuffle(possibilities, possibilities + sampleStructure.featureCount(), g);

        //Now take the first few.

        unsigned numToTake = (unsigned)(filterCutoff * sampleStructure.featureCount());

        for(unsigned i = 0; i < numToTake; i++){
          filter[possibilities[i]] = true;
          if(verbose) std::cerr << "Randomly Dropping Feature " << possibilities[i] << " \"" << sampleStructure.featureName(possibilities[i]) << "\"." << std::endl;
        }
      
        break;
      }
      case filter_entropy:
        filter = entropyFilterBools(sampleStructure, filterCutoff, training, verbose);
        break;
      
      case filter_entropy_transductive:  
        filter = entropyFilterBoolsTransductive(sampleStructure, filterCutoff, training, query, verbose);
        break;
        
      case filter_klDivergence:
        filter = klDivergenceFilterBools(sampleStructure, filterCutoff, training, query, verbose);
        break;
    }
    
    frac = new pfFRaC_G(sampleStructure, p, e, filter);
  }
  else {
    frac = new TraditionalFRaC_G(sampleStructure, p, e, useUnknownCorrection);
  }
  
  if(verbose){
    std::cerr << "Training complete." << std::endl;
  }
  
  //Run FRaC with it.
  
  Array<SampleScorePair> results;
  if(logTermScores){
    std::ofstream logFile;
    openOutputFile(logFile, termScoreLogFile);
    results = frac->scoreTestSet(query, logFile);
    logFile.close();
  }
  else {
    results = frac->scoreTestSet(query);
  }
  
  switch (inputFormat){
    case input_ARFF:
    {
      
      std::ofstream outFile;
      if(ioFilePaths[OUTFILE] != 0){
        openOutputFile(outFile, ioFilePaths[OUTFILE]);
      }
      
      std::ostream& out = (ioFilePaths[OUTFILE] == 0) ? std::cout : outFile;
      
      if(outMode == arff_with_anom_score_sorted){
        results.sort();
      }

      if(outMode == anom_score)
      {
        for(unsigned i = 0; i < results.length; i++){
          std::cout << results[i].anomalyScore << std::endl;
        }
      } else {
        writeArff(std::cout, sampleStructure, results);
      }
      
      if(ioFilePaths[OUTFILE] != 0){
        outFile.close();
      }
      
      break;
    }
    case input_FRaC_test:
    {

      //Write (class, anomaly score) pairs for AUROC evaluation.
      std::ofstream out;

      openOutputFile(out, std::string(ioFilePaths[TRAINSET]) + "/frac_out");

      for (unsigned i = 0; i < results.length; i++) {
        out << (testClasses[i] == true ? 0 : 1) << "\t" << results[i].anomalyScore << std::endl;
      }

      out.close();
      
      //Write each sample (minus class info) with anomaly scores.
      openOutputFile(out, std::string(ioFilePaths[TRAINSET]) + "/frac_out.arff");
      
      writeArff(out, sampleStructure, results);
      
      out.close();

      //Write a log of the error models.
      openOutputFile(out, std::string(ioFilePaths[TRAINSET]) + "/frac_log");
      
      out << "Regressors (feature, index):" << std::endl;
      for(unsigned i = 0; i < sampleStructure.contvarCount; i++){
        out << "Continuous Feature " << i << ", (differential) entropy = " << *e.continuousFeatureEntropy(i) << std::endl;
        for(unsigned j = 0; j < p.regressorCount; j++){
          out << "Regressor " << j << ": " << std::endl;
          out << *p.getRegressor(i, j) << std::endl;
          out << *e.getContinuousModel(i, j) << std::endl;
        }
        out << std::endl;
      }

      out << "Classifiers (feature, index):" << std::endl;
      for(unsigned i = 0; i < sampleStructure.catvarCount; i++){
        out << "Categorical Feature " << i << ", entropy = " << *e.categoricalFeatureEntropy(i) << std::endl;
        for(unsigned j = 0; j < p.classifierCount; j++){
          out << "Classifier " << j << ": " << std::endl;
          out << *p.getClassifier(i, j) << std::endl;
          out << *e.getCategoricalModel(i, j) << std::endl;
        }
      }
      
      out << "Binary Classifiers (feature, index):" << std::endl;
      for(unsigned i = 0; i < sampleStructure.binvarCount; i++){
        out << "Binary Feature " << i << ", entropy = " << *e.binaryFeatureEntropy(i) << std::endl;
        for(unsigned j = 0; j < p.binaryClassifierCount; j++){
          out << "Binary Classifier: " << j << std::endl;
          out << *p.getBinaryClassifier(i, j) << std::endl;
          out << *e.getBinaryModel(i, j) << std::endl;
        }
      }

      out.close();
    }
  }
  
  if(verbose){
    std::cerr << "FRaC complete." << std::endl;
  }
  
  
  //Free data.
  //Note: The operating system can handle this, this is just for accounting purposes.
  
  #ifdef DEBUG
    
  switch(inputDataUsage){
    case mode_withValidationSet:
      training.forEach([](Sample& s){s.freeMemory();});
      training.freeMemory();
      
      validation.forEach([](Sample& s){s.freeMemory();});
      validation.freeMemory();
      
      query.forEach([](Sample& s){s.freeMemory();});
      query.freeMemory();
      break;
    case mode_crossValidation:
      training.forEach([](Sample& s){s.freeMemory();});
      training.freeMemory();
      
      query.forEach([](Sample& s){s.freeMemory();});      
      query.freeMemory();
      break;
    case mode_unsupervised:
      query.forEach([](Sample& s){s.freeMemory();});
      query.freeMemory();
  }
  
  switch(inputFormat){
    case input_FRaC_test:
      //Memory: This could be freed earlier.
      testClasses.freeMemory();
      break;
    default:
      break;
  }
  
  //Free the results;
  
  //The raw data in the results are from the query set, so they are already freed.
  //results.forEach([](SampleScorePair ssp){ssp.sample.freeData();});
  results.freeMemory();
  
  frac->freeMemory(); //Responsible for freeing error model and predictor.
  delete frac;
  
  mlc.freeMemory(); //Note: could free earlier.
  
  sampleStructure.freeData();
  
  #endif
  
	return 0;

}

static void exit_with_version() { 
	fprintf(stderr, "%s version %s\n", PROGRAM, VERSION); 
	exit(0);
}

static void exit_with_help() 
{
	fprintf(stderr,

	"\n"
	"\n"
	"This is FRaC with SVR feature models, decision trees, and Gaussian error models.\n"
	"All SVR code is from LIBSVM version 3.1 (See README and COPYRIGHT).\n" 
  "Decision tree code is from Waffles.\n"
  "\n"
	"\n"
	"Usage: %s [options]\n"
	"\n"
	"  Program Options:\n"
	"\n"
	"    -h Print this help message and exit.\n"
	"    -v Print version information.\n"
  "    -l Loud (verbose) mode.  Prints status updates to stderr.\n"
	"\n"
	"  FRaC options:\n"
	"  \n"
  "    -C Select continuous error model type.\n"
  "     0 -- Gaussian error models.\n"
  "     1 -- N bin gaussian kernel, sigma = sqrt(N) ^ -1.\n"
  "     2 -- sqrt(N) bin gaussian kernel, unweighted bins spaced for even sample distribution, sigma = bin width.\n"
  "     3 -- sqrt(N) bin gaussian kernel, even sized weighted bins, sigma = bin width.\n"
  "    -E Use FRaC evaluation input (which requires provision of a directory containing trainset, testset, and metadata files).  Defaults to ARFF.\n"
  "    -M <integer> Use an ensemble of classifiers and regressors.\n"
  "     0 -- inner ensemble (large).\n"
  "     1 -- outer ensemble (large).\n"
  "     2 -- inner ensemble of 2.\n"
  "     3 -- outer ensemble of 2.\n"
  "    -U Use unknown value correction.\n"
  "    -D Use division FRaC.\n"
  "    -Z Use naive dFRaC.\n"
	"    -X <string> Input file (required; arff format).\n"
	"    -Q <string> Test file (for semisupervised anomaly detection).  If missing, do mode_unsupervised.\n"
	"    -V <string> Validation file (for building error models).  If missing, do cross-validation.\n"
  "    -O <string> Output File.  If missing, stdout is used.\n"
  "    -F <string> Set output mode (ARFF mode only).\n"
  "      \"arffsort\": ARFF file of test data, augmented with anomaly scores (default).\n"
  "      \"arff\": Unsorted ARFF file (in order of test set), augmented with anomaly scores.\n"
  "      \"raw\": Newline delimited anomaly scores, in order of test set.\n"
  "    -N <integer> N-fold cross-validation for building error models (omit to do leave-one-out).\n"
  "    -S <integer> Number of shuffles to to use in cross validation.\n"
  "    -L <string> Log termwise anomaly scores.\n"
  "\n"
  "  Heuristic Optimization Options\n"
  "    -K <real> Filter fraction.\n"
  "    -R filter_type: Change the type of the categorical value filter.\n"
  "     0 -- Random\n"
  "     1 -- Low entropy (training only)\n"
  "     2 -- Low entropy (transduction)\n"
  "     3 -- KL divergence (transduction)\n"
  "     -P Use partial filtering (with filter options, in place of full filter)\n"
  "\n"
  "  Waffles Options:\n"
	"    -f <integer> Random forest tree count, 0 to disable (default 0).\n"
  "    -B <integer> Minimum number of samples a bucket must have to branch in a decision tree (default 0).\n"
  "\n"
	"  LIBSVM options:\n"
	"  \n"
  "    -T Timeout (seconds) for SVM solver optimization (default 86400, or 24:00:00).\n"
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
  "    One is required to provide any one of the following:\n"
  "      Training Set, Validation Set, Query Set -> FRaC with validation set\n"
  "      Training Set, Query Set -> FRaC with cross validation\n"
  "      Training Set -> Unsupervised FRaC\n"
  "\n"
  "    If these options are not provided with the -X -Q -V -O flags, they may be provided as flagless arguments, in the order {training, validation, query, output}.\n"
          
	,PROGRAM
	);
	exit(1);
}

static void parse_command_line(int argc, char **argv)
{
  
  //Default to stdout
	void (*print_func)(const char*) = [](const char* s){
    fputs(s,stdout);
    fflush(stdout);
  };

	// parse options
	for (int i = 1; i < argc; i++) {
		
    //Non flag arguments are used to set the input files
		if (argv[i][0] != '-') {
      for(unsigned j = 0; j < FILEPATHCOUNT; j++){
        if(ioFilePaths[j] == NULL){
          ioFilePaths[j] = argv[i];
          goto endLoopIter; //Jump to the next iteration
        }
      }
      //We get to this block iff all file path slots are full.
      {
        std::cerr << "Error: Only " << FILEPATHCOUNT << " raw (non flag) arguments can be provided: Train Set, Validation Set, Query Set, and Output Set";
        exit_with_help();
      }
    }

		// noto, check options that do not require an argument
		switch(argv[i][1]) { 

			case 'v': exit_with_version();
			case 'h': exit_with_help();
      
      //Input Format
      case 'E':
        inputFormat = input_FRaC_test;
        continue;
      case 'U':
        useUnknownCorrection = true;
        continue;
			case 'q':
				print_func = [](const char* c){};
				continue;
        
      case 'D':
        useDivisionFRaC = true;
        continue;
      case 'Z':
        useNaiveDivisionFRaC = true;
        continue;
      case 'l':
        verbose = true;
        continue;
        
      case 'P':
        usepfFRaC = true;
        continue;
        
		}

		// noto, all other options require an argument.  advance 'i' to position of argument
		if (++i >= argc) { exit_with_help(); }

		switch(argv[i-1][1]) {

			//INPUT FILES
			case 'X':
				ioFilePaths[TRAINSET] = argv[i]; 
				break;
			case 'V':
				ioFilePaths[VALISET] = argv[i]; 
				break;
			case 'Q':
				ioFilePaths[QUERYSET] = argv[i]; 
				break;
			case 'O':
				ioFilePaths[OUTFILE] = argv[i]; 
				break;
        
      //Output Format
      case 'F':
        if(std::string(argv[i]) == "arffsort")
          outMode = arff_with_anom_score_sorted;
        else if (std::string(argv[i]) == "arff")
          outMode = arff_with_anom_score_unsorted;
        else if (std::string(argv[i]) == "raw")
          outMode = anom_score;
        else{
          std::cerr << "Unrecognized output mode \"" << argv[i] << "\".  Ignoring." << std::endl;
        }
        break;
      
      //FRAC OPTIONS
			case 'N':
				cvFolds = readUnsignedSafe(argv[i], argv[i-1]);
        if(cvFolds == 1){
          std::cerr << "Invalid -" << argv[i-1][1] << " option: Must specify > 1 folds for cross validation, or 0 for LOOCV." << std::endl;
				  exit(1);
        }
        break;
      case 'S':
        cvShuffles = readUnsignedSafe(argv[i], argv[i-1]);
        break;
			case 'T':
				svm_param.timeout = readUnsignedSafe(argv[i], argv[i-1]);
				break;
      case 'L':
        logTermScores = true;
        termScoreLogFile = argv[i];
        break;
        
      //Heuristic Options:
      case 'K':
        filterCutoff = readFloatSafe(argv[i], argv[i-1], 0, 1);
        break;
        
      case 'R':
        filterMode = readEnumSafe(FilterMode, FilterMode_LENGTH, argv[i], argv[i-1]);
        continue;
        
      //Ensemble options:
      case 'M':
        ensembleType = readEnumSafe(EnsembleType, EnsembleType_LENGTH, argv[i], argv[i-1]);
        break;
        
      //Waffles options
      case 'f': //Random Forest:
        randomForestSize = readUnsignedSafe(argv[i], argv[i-1]);
        break;
      case 'B': //Min decision tree bucket size
        decisionTreeBucketMin = readUnsignedSafe(argv[i], argv[i-1]);
        break;

			//SVM OPTIONS
			case 's':
        //Detail: Replace theese atoi and atof with the read functions defined above.
        //Detail: This is confusing error handling.
				svm_param.svm_type = atoi(argv[i]) + EPSILON_SVR; // EPSILON_SVR is the 0th feasible choice for this 
				if (svm_param.svm_type != EPSILON_SVR && svm_param.svm_type != NU_SVR) { fprintf(stderr, "\nIllegal option: -s %d\n\n", svm_param.svm_type); exit_with_help(); } // noto
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
			// 'w' weight option removed
        
        
      //Error model options
      
      case 'C':
        contErrorModelType = readEnumSafe(ContErrorModelType, ContErrorModelType_LENGTH, argv[i], argv[i-1]);
        break;
      
			default:
				fprintf(stderr,"Unknown option: -%c\n", argv[i-1][1]);
				exit_with_help();
		}
    endLoopIter:;
    
	}

  //Check input and determine run mode.
  
  if(inputFormat == input_ARFF){

    //Must have training data
    if (!ioFilePaths[TRAINSET]){
      fprintf(stderr, "Missing training set\n");
      exit_with_help();
    }
    if(ioFilePaths[VALISET] && ioFilePaths[QUERYSET]){
      inputDataUsage = mode_withValidationSet;
    } else if (!ioFilePaths[VALISET]){
      if(!ioFilePaths[QUERYSET]){
        inputDataUsage = mode_unsupervised;
      }
      else{
        inputDataUsage = mode_crossValidation;
      }
    }
    else{
      std::cerr << "Please provide a valid combination of test, validation, and query data." << std::endl;
      exit_with_help();
    }
  } else{ //FRaC evaluation format input
    if(!ioFilePaths[TRAINSET]){
      fprintf(stderr, "Missing training set\n");
      exit_with_help();
    }
  }
  
	svm_set_print_string_function(print_func);

}
