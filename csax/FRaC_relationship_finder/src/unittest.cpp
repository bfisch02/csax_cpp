
#include <iostream>
#include <sstream>

#include "waffles.hpp"

#include "predictor.hpp"
#include "frac.hpp"

#include "errormodel.hpp"
#include "errormodelcollection.hpp"
#include "io.hpp"

#include "svm.h"
#include "svmlearner.hpp"
#include "vectormath.hpp"

/////////////
//TEST DATA//
/////////////

//In this test data, contvar 0, 1 proportional to catvar 0, 1 respectively.  contvar 2 random.  catvar 2 = catvar 0 + catvar 1, catvar 3 = catvar 0 * catvar 1

#define TRAININGSIZE 32

contvar_t trainingContvars[TRAININGSIZE][3] = {
  //Good data
  {0, 0, 0},
  {1, 1, 1},
  {2.1, 2.2, 2.3},
  {0, 1, 2},
  
  {1, 2, 1},
  {1, 3, 3},
  {2, 3, 5},
  {2, 2, 7},
  
  {1.2, 1.9, -1.3},
  {.5, 2.5, 2.6},
  {1.25, 3, 12.3},
  {2.125, 1.9, 4.8},
  
  {.125, 3.4, -1.2},
  {-.5, .3, .01},
  {2.3, .2, .2},
  {2.6, .1, -.5},
  
  {2.1, .4, 0},
  {2.1, 3.4, -1.3},
  {2.9, 1.1, 12.55},
  {2.1, 1.2, 18.4},
  
  {2.3, 2.2, -11},
  {0.4, 1.3, 2.6},
  {1.2, 2.2, 5.6},
  {.9, 3.15, -1.8},
  
  {2.1, 2.8, 0},
  {2.3, 1.84, .17},
  {1.9, 0.9, 11},
  {2.2, 2.1, 2.0},
  
  {0.3, 3.2, -12.34},
  {2.1, -0.1, 1.4},
  {1.4, 2.6, -.4},
  {2.55, 1.2, -.6}
};

catvar_t trainingCatvars[TRAININGSIZE][4] = {
  //Good data
  {1, 0, 1, 0},
  {0, 1, 1, 0},
  {2, 3, 5, 6},
  {1, 1, 2, 1},
  
  {0, 3, 3, 0},
  {0, 2, 2, 0},
  {2, 2, 4, 4},
  {2, 3, 5, 6},
  
  {0, 3, 3, 0},
  {0, 2, 2, 0},
  {2, 2, 4, 4},
  {2, 3, 5, 6},
  
  {1, 2, 3, 2},
  {1, 0, 1, 0},
  {2, 0, 2, 0},
  {2, 0, 2, 0},
  
  {2, 1, 3, 2},
  {2, 2, 4, 4},
  {2, 1, 3, 2},
  {2, 1, 3, 2},
  
  {0, 3, 3, 0},
  {1, 1, 2, 1},
  {0, 3, 3, 0},
  {0, 2, 2, 0},
  
  {2, 2, 4, 4},
  {2, 3, 5, 6},
  {2, 1, 3, 2},
  {2, 3, 5, 6},
  
  {1, 2, 2, 3},
  {2, 0, 2, 0},
  {0, 2, 2, 0},
  {2, 1, 3, 2}
};

Array<Sample> getTrainingSampleData(){
  unsigned sampleCount = TRAININGSIZE;
  unsigned noiseCount = 0; //TODO
  
  Sample* data = new Sample[sampleCount + noiseCount];
  
  //Written data
  for(unsigned i = 0; i < sampleCount; i++){
    data[i] = Sample(trainingContvars[i], trainingCatvars[i], std::vector<bool>(), std::vector<unsigned>());
  }
  
  //Noise data (TODO)
  for(unsigned i = 0; i < noiseCount; i++){
    //data[i + sampleCount] = ...
  }
  
  Array<Sample> testData = Array<Sample>((sampleCount + noiseCount), data);
  return testData;
}

#define EVALUATIONSIZE 32

contvar_t evaluationContvars[EVALUATIONSIZE][3] = {
  //Good data
  {1, 0, 0},
  {0, 1, 1},
  {2, 3, 2},
  {1, 1, 2},
  
  {0, 3, 1},
  {0, 2, 3},
  {2, 2, 5},
  {2, 3, 7},
  
  {0.2, 3.9, -11.3},
  {.5, 3.4, 26.6},
  {0.25, 2, 152.3},
  {2.125, 28.9, 4.8},
  
  {1.25, 2.4, -21.2},
  {1.1, .3, 1.01},
  {2.3, .2, 5.2},
  {2.6, .1, -6.5},
  
  {2.1, .4, 70},
  {2.1, 2.4, -41.3},
  {2.9, 1.1, 712.55},
  {2.1, 1.2, 318.4},
  
  {2.3, 3.2, -711},
  {1.4, 1.3, 92.6},
  {0.2, 3.2, 75.6},
  {.1, 2.15, -41.8},
  
  {2.1, 1.8, 90},
  {2.3, 2.84, 3668.17},
  {1.9, 0.9, 911},
  {2.2, 3.1, 52.0},
  
  {1.3, 2.2, -912.34},
  {2.1, -0.1, 551.4},
  {0.4, 2.1, -2.4},
  {2.55, 1.2, -158483.6}
};

catvar_t evaluationCatvars[EVALUATIONSIZE][4] = {
  //Good data
  {0, 0, 0, 0},
  {1, 1, 2, 1},
  {2, 2, 4, 4},
  {0, 1, 1, 0},
  
  {1, 2, 3, 2},
  {1, 3, 4, 3},
  {2, 3, 5, 6},
  {2, 2, 4, 4},
  
  {1, 2, 3, 2},
  {1, 3, 4, 3},
  {2, 3, 5, 6},
  {2, 2, 4, 4},
  
  {0, 3, 3, 0},
  {0, 0, 0, 0},
  {2, 0, 2, 0},
  {2, 0, 2, 0},
  
  {2, 1, 3, 2},
  {2, 3, 5, 6},
  {2, 1, 3, 2},
  {2, 1, 3, 2},
  
  {1, 2, 3, 2},
  {0, 1, 1, 0},
  {1, 2, 3, 2},
  {1, 3, 4, 3},
  
  {2, 3, 5, 6},
  {2, 2, 4, 4},
  {2, 1, 3, 2},
  {2, 2, 4, 4},
  
  {0, 3, 3, 0},
  {2, 0, 2, 0},
  {1, 3, 4, 3},
  {2, 1, 3, 2}
};

Array<Sample> getEvaluationSampleData(){
  unsigned sampleCount = EVALUATIONSIZE;
  unsigned noiseCount = 0; //TODO
  
  Sample* data = new Sample[sampleCount + noiseCount];
  
  //Written data
  for(unsigned i = 0; i < sampleCount; i++){
    data[i] = Sample(evaluationContvars[i], evaluationCatvars[i], std::vector<bool>(), std::vector<unsigned>());
  }
  
  //Noise data (TODO)
  for(unsigned i = 0; i < noiseCount; i++){
    //data[i + sampleCount] = ...
  }
  
  Array<Sample> evaluationData = Array<Sample>((sampleCount + noiseCount), data);
  return evaluationData;
}

#define TESTSIZE 8

contvar_t testContvars[TESTSIZE][3] = {
  {1.25, 2.4, 11.8},
  {.75, 3.2, 4.5},
  {2.2, 3.1, 101.2},
  {2.2, 1.8, -111.8},
  
  {0.3, 0.9, -0.5},
  {0.15, 0.1, -1274.5},
  {2.2, 1.1, 11.2},
  {1.2, 2.8, -1.8}
};

catvar_t testCatvars[TESTSIZE][4] = {
  //Good data
  {1, 2, 3, 2},
  {1, 3, 4, 3},
  {2, 3, 5, 6},
  {2, 2, 4, 4},
  
  {0, 1, 1, 0},
  {0, 0, 0, 0},
  {2, 1, 3, 2},
  {1, 3, 4, 3}
  
};

Array<Sample> getTestSampleData(){
  unsigned sampleCount = TESTSIZE;
  
  Sample* data = new Sample[sampleCount];
  
  //Written data
  for(unsigned i = 0; i < sampleCount; i++){
    data[i] = Sample(testContvars[i], testCatvars[i], std::vector<bool>(), std::vector<unsigned>());
  }
  
  Array<Sample> testData = Array<Sample>(sampleCount, data);
  return testData;
}

#define ANOMSIZE 6

contvar_t anomContvars[ANOMSIZE][3] = {
  {2.25, 2.4, 1.8},
  {1.75, 3.2, .5},
  {1.2, 3.1, 10.2},
  {0.2, 3.8, -11.8},
  {0.3, 1.9, -0.5},
  {0.15, 2.1, -174.5}
};

catvar_t anomCatvars[ANOMSIZE][4] = {
  //Good data
  {1, 2, 2, 1},
  {1, 3, 1, 1},
  {2, 3, 5, 5},
  {2, 2, 1, 0},
  {0, 1, 2, 3},
  {0, 0, 4, 1}
};


Array<Sample> getAnomalousSampleData(){
  unsigned sampleCount = ANOMSIZE;
  
  Sample* data = new Sample[sampleCount];
  
  //Written data
  for(unsigned i = 0; i < sampleCount; i++){
    data[i] = Sample(anomContvars[i], anomCatvars[i], std::vector<bool>(), std::vector<unsigned>());
  }
  
  Array<Sample> testData = Array<Sample>(sampleCount, data);
  return testData;
}

std::string structureNames[7] = {"cont A", "cont B", "noise", "Cat A", "Cat B", "Cat A + Cat B", "Cat A * Cat B"};
unsigned structureCatvarSizes[4] = {3, 4, 6, 7}; //Somewhere a 12 is getting put in the third category.  I have no idea how, memory corruption of some sort?
//unsigned structureCatvarSizes[4] = {8, 8, 12, 8}; //TODO fix the bug in the classifier that is causing this.
SampleStructure getTestSampleStructure(){
  SampleStructure st(3, 4, 0, structureCatvarSizes, structureNames, "Test_Relation");

  //std::cout << "ST: " << st << std::endl;
  *st.catvarCatNameLookup(0, 0) = "c0";
  *st.catvarCatNameLookup(0, 1) = "c1";
  *st.catvarCatNameLookup(0, 2) = "c2";
  
  *st.catvarCatNameLookup(1, 0) = "c0";
  *st.catvarCatNameLookup(1, 1) = "c1";
  *st.catvarCatNameLookup(1, 2) = "c2";
  *st.catvarCatNameLookup(1, 3) = "c3";

  *st.catvarCatNameLookup(2, 0) = "c0";
  *st.catvarCatNameLookup(2, 1) = "c1";
  *st.catvarCatNameLookup(2, 2) = "c2";
  *st.catvarCatNameLookup(2, 3) = "c3";
  *st.catvarCatNameLookup(2, 4) = "c4";
  *st.catvarCatNameLookup(2, 5) = "c5";

  *st.catvarCatNameLookup(3, 0) = "c0";
  *st.catvarCatNameLookup(3, 1) = "c1";
  *st.catvarCatNameLookup(3, 2) = "c2";
  *st.catvarCatNameLookup(3, 3) = "c3";
  *st.catvarCatNameLookup(3, 4) = "c4";
  *st.catvarCatNameLookup(3, 5) = "c5";
  *st.catvarCatNameLookup(3, 6) = "c6";
  
  st.contvarMeans = new contvar_t[st.contvarCount];
  
  //std::cout << "TEST SS: Feature Counts: " << st.contvarCount << ", " << st.catvarCount << ", " << st.binvarCount << std::endl;
  //exit(0);
  
  return st;
}

///////////////////
//GENERAL TESTING//
///////////////////

void validateTestData(){
  
  std::cout << "Validating Test Dataset." << std::endl;
  
  StructuredSampleCollection col;
  SampleStructure st = getTestSampleStructure();
  Array<Sample> data;
  
  data = getTrainingSampleData();
  col = StructuredSampleCollection(st, data);
  
  std::cout << "Training Data: " << col.validate() << std::endl;
  
  data = getTestSampleData();
  col = StructuredSampleCollection(st, data);
  
  std::cout << "Test Data: " << col.validate() << std::endl;
  
  data = getAnomalousSampleData();
  col = StructuredSampleCollection(st, data);
  
  std::cout << "Anomalous Data: " << col.validate() << std::endl;
  
}

///////////////////
//WAFFLES TESTING//
///////////////////

void testWafflesLoading(){
  
  std::cout << "Testing Waffles Loading.\n";
  
  //Create sample type
  SampleStructure st = getTestSampleStructure();
  
  Array<Sample> training = getTrainingSampleData();
  training.data += 2; //We are curious about the third sample.
  training.length = 1;
  
  std::cout << "Training Sample 0: " << StructuredSample(st, training[0]) << std::endl; 
 
  for(unsigned featureIndex = 0; featureIndex < st.catvarCount; featureIndex++){
    GClasses::GMatrix featureSchema = generateCategoricalFeatureSchema(st, featureIndex);
    GClasses::GMatrix labelSchema = generateCategoricalLabelSchema(st, featureIndex);

    loadClassifierTrainingData(featureSchema, labelSchema, st, featureIndex, training);

    std::cout << "Label " << featureIndex << ": " << labelSchema.row(0)[0] << ", Features : " << Array<double>(featureSchema.row(0), featureSchema.cols()) << std::endl;
  }
  
}

void testWafflesDT(){
  
  std::cout << "Testing Waffles Decision Tree.\n";
  
  //Create sample type
  SampleStructure st = getTestSampleStructure();
  
  std::cout << "Sample Structure: " << st << "\n";

  //Create training data
  Array<Sample> trainingData = getTrainingSampleData();
  
  std::cout << "Training: ";
  writeSampleArr(std::cout, trainingData, st); //TODO SampleSet?  Combine Sample and SampleType.
  std::cout << "\n";
  
  
  //Make some test data
  Array<Sample> testData = getTestSampleData();
  
  std::cout << "Test: ";
  writeSampleArr(std::cout, testData, st);
  
  //Make some decision trees
  Classifier** trees = new Classifier*[st.catvarCount];
  //Version for no default constructors.
  //char treemem [sizeof(WafflesDecisionTree) * 4];
  //WafflesDecisionTree* trees = (WafflesDecisionTree*)treemem;
  
  for(unsigned i = 0; i < st.catvarCount; i++){
    
    std::cout << "Training tree " << (double)i << ".\n";
    
    trees[i] = new WafflesDecisionTreeClassifier(st, i, true);
    trees[i]->train(trainingData);
    
  }
  
  //Run some tests
  
  unsigned errors = 0;
  
  for(unsigned s = 0; s < testData.length; s++){
    std::cout << "SAMPLE " << ((double)s) << ": ";
    for(unsigned f = 0; f < st.contvarCount; f++){
      std::cout << testData[s].contvars[f] << ", ";
    }
    for(unsigned f = 0; f < st.catvarCount; f++){
      std::cout << ((double)testData[s].catvars[f]) << ", ";
    }
    std::cout << "\nPREDICTIONS: ";
    for(unsigned f = 0; f < st.catvarCount; f++){
      catvar_t prediction = trees[f]->predict(testData[s]);
      std::cout << ((double)prediction) << ", ";
      if(prediction != testData[s].catvars[f]){
        errors++;
      }
    }
    std::cout << "\n";
  }
  std::cout << "Accuracy: " << (1.0 - ((double)errors) / (testData.length * st.catvarCount)) << ".\n";

  delete [] trees;
}

Predictor getTestPredictorWaffles(){
  
  //Load sample structure
  SampleStructure sampleStructure = getTestSampleStructure();
  
  //Load training data
  Array<Sample> trainingData = getTrainingSampleData();
  
  //Make creator lambdas
  MlCreators creators;
  
  //Lambdas and side effects are best friends!
  creators.rc = new RegressorCreator[1];
  creators.rc[0] = [](SampleStructure* st, Array<Sample> training, unsigned index){
    //Regressor* r = new ConstantRegressor(0);
    Regressor* r = new WafflesDecisionTreeRegressor(*st, index);
    r->train(training);
    return r;
  };
  creators.cc = new ClassifierCreator[1];
  creators.cc[0] = [](SampleStructure* st, Array<Sample> training, unsigned index){
    Classifier* c = new WafflesDecisionTreeClassifier(*st, index, true); //TODO Why does this random give identical results to the default?
    c->train(training);
    
    return c;
  };
  creators.bcc = new BinaryClassifierCreator[1];
  creators.bcc[0] = [](SampleStructure* st, Array<Sample> training, unsigned index){
    BinaryClassifier* r = new ConstantBinaryClassifier(false);
    r->train(training);
    return r;
  };
  
  //TODO build + use takeBest.
  
  //std::cout << "Making predictor." << std::endl;
  Predictor p = Predictor(sampleStructure, trainingData, creators);
  
  //std::cout << "Made predictor." << std::endl;
  
  return p;
}

Predictor getTestPredictorLibSvm(){
  
  //Load sample structure
  SampleStructure sampleStructure = getTestSampleStructure();
  
  //Load training data
  Array<Sample> trainingData = getTrainingSampleData();
  
  //Make creator lambdas
  MlCreators creators;
  
   creators.rc = new RegressorCreator[1];
   creators.rc[0] = [](SampleStructure* st, Array<Sample> training, unsigned index){
    
    svm_parameter svm_param;
    
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

    Regressor* r = new SvmRegressor(*st, index, &svm_param);
    r->train(training);
    return r;
  };
  
  creators.cc = new ClassifierCreator[1];
  creators.cc[0] = [](SampleStructure* st, Array<Sample> training, unsigned index){
    Classifier* c = new WafflesDecisionTreeClassifier(*st, index, true);
    c->train(training);
    
    return c;
  };
  
  creators.bcc = new BinaryClassifierCreator[1];
  creators.bcc[0] = [](SampleStructure* st, Array<Sample> training, unsigned index){
    BinaryClassifier* r = new ConstantBinaryClassifier(false);
    r->train(training);
    return r;
  };
  
  //TODO build + use takeBest.
  
  Predictor p = Predictor(sampleStructure, trainingData, creators);
  
  return p;
}

#if 0
void testPredictor(Predictor p){
  
  //Load sample structure
  SampleStructure sampleStructure = getTestSampleStructure();
  
  //Load test data
  Array<Sample> testData = getTestSampleData();
  
  unsigned errors = 0;
  
  for(unsigned s = 0; s < testData.length; s++){
    
    Sample predictedSample = p.predict(testData[s]);
    
    std::cout << "SAMPLE " << ((double)s) << ": ";
    for(unsigned f = 0; f < sampleStructure.contvarCount; f++){
      std::cout << testData[s].contvars[f] << ", ";
    }
    for(unsigned f = 0; f < sampleStructure.catvarCount; f++){
      std::cout << ((double)testData[s].catvars[f]) << ", ";
    }
    std::cout << "\nPREDICTIONS: ";
    for(unsigned f = 0; f < sampleStructure.contvarCount; f++){
      contvar_t prediction = predictedSample.contvars[f]; //p.classifiers[f]->predict(testData[s]);
      std::cout << ((double)prediction) << ", ";
      //if(prediction != testData[s].catvars[f]){
      //  errors++;
      //}
    }for(unsigned f = 0; f < sampleStructure.catvarCount; f++){
      catvar_t prediction = predictedSample.catvars[f]; //p.classifiers[f]->predict(testData[s]);
      std::cout << ((double)prediction) << ", ";
      if(prediction != testData[s].catvars[f]){
        errors++;
      }
    }
    std::cout << "\n";
  }
  std::cout << "Catvar Accuracy: " << (1.0 - ((double)errors) / (testData.length * sampleStructure.catvarCount)) << ".\n";
  
}

void testFrac(Predictor p){
  
  std::cout << "Testing FRaC.\n";
  
  SampleStructure st = getTestSampleStructure();
  
  Array<Sample> evaluationData = getEvaluationSampleData();
  
  ErrorModelCreators emc;
  emc.rc = [](SampleStructure* st, unsigned index, contvar_t* trueData, contvar_t* predictedData, unsigned length){
    return (ContinuousErrorModel*) new Gaussian(trueData, predictedData, length);
  };
  emc.cc = [](SampleStructure* st, unsigned index, catvar_t* trueData, catvar_t* predictedData, unsigned length){
    return (CategoricalErrorModel*) new SurprisalMatrix(st->catvarSizes[index], trueData, predictedData, length);
  };
  
  emc.bcc = [](SampleStructure* st, unsigned index, binvar_t* trueData, binvar_t* predictedData, unsigned length){
    Array<catvar_t> trueConv = Array<binvar_t>(trueData, length).map<catvar_t>([](binvar_t bv){return (catvar_t)bv;});
    Array<catvar_t> predConv = Array<binvar_t>(predictedData, length).map<catvar_t>([](binvar_t bv){return (catvar_t)bv;});

    SurprisalMatrix catmodel = SurprisalMatrix(2, trueConv.data, predConv.data, length);
    
    trueConv.freeMemory();
    predConv.freeMemory();
    
    return (BinaryErrorModel*) new BinaryErrorModelFromCategoricalErrorModel<SurprisalMatrix>(SurprisalMatrix());
  };
  //Note: Here we build error models on training data: we should be using new validation data.
  
  ErrorModelCollection e = ErrorModelCollection(st, emc, evaluationData, p);
  
  /*
  gaussian* rem = new gaussian[st.contvarCount];
  NormalizedSurprisalMatrix* cem = new NormalizedSurprisalMatrix[st.catvarCount];
  for(unsigned i = 0; i < st.catvarCount; i++){
    
    unsigned length = trainingData.length;
    catvar_t* trueData = new catvar_t[length];
    catvar_t* predictedData = new catvar_t[length];
    
    for(unsigned j = 0; j < length; j++){
      trueData[j] = trainingData[j].catvars[i];
      predictedData[j] = p.classifiers[i]->predict(trainingData[j]);
    }
    
    cem[i] = NormalizedSurprisalMatrix(st.catvarSizes[i], trueData, predictedData, length);
  }
  char* bcem = new char[st.binvarCount];
  
  ErrorModelCollection e = ErrorModelCollection(rem, cem, bcem);
   */
  
  
  TraditionalFRaC frac(st, p, e);
  
  Array<Sample> testData = getTestSampleData();
  
  for(unsigned i = 0; i < testData.length; i++){
    double ns = frac.calculateNS(testData[i], p.predict(testData[i]));

    std::cout << "Normal Sample " << i << ": NS = " << ns << "\n";
    std::cout << StructuredSample(st, testData[i]) << "\n";
  }
  
  Array<Sample> anomTestData = getAnomalousSampleData();
  
  for(unsigned i = 0; i < anomTestData.length; i++){
    double ns = frac.calculateNS(anomTestData[i], p.predict(testData[i]));

    std::cout << "Anomalous Sample " << i << ": NS = " << ns << "\n";
    std::cout << StructuredSample(st, anomTestData[i]) << "\n";
  }

}
#endif

void testGaussianNumerics(){
  
  std::cout.precision(16);
  std::cout << std::scientific;
    
  Gaussian g = Gaussian(5, 1);
  for(unsigned i = 0; i < 100; i++){
    std::cout << i << ": L: " << g.likelihood((fracfloat_t)i) << " -LL: " << (-INV_LN_2 * log(g.likelihood(i))) << ", S: " << g.surprisal((fracfloat_t)i) << std::endl;
  }
}

void testNormalizedSurprisal(){
  
  std::cout << "Normalized Surprisal Matrix tests.\n";

  unsigned classCount = 2;
  
  #define NSTEST1SIZE 4
  catvar_t trueData[NSTEST1SIZE] = {0, 0, 1, 1};
  catvar_t predictedData[NSTEST1SIZE] = {0, 1, 0, 1};
  
  std::cout << "\n";
  
  SurprisalMatrix ns = SurprisalMatrix(classCount, trueData, predictedData, NSTEST1SIZE);
  
  std::cout << ns;
  
  
  #define NSTEST2SIZE 4
  catvar_t trueData2[NSTEST2SIZE] = {0, 1, 1, 1};
  catvar_t predictedData2[NSTEST2SIZE] = {0, 1, 0, 1};
  
  
  SurprisalMatrix ns2 = SurprisalMatrix(classCount, trueData2, predictedData2, NSTEST2SIZE);
  
  std::cout << ns2;
}

void testArff(){
  StructuredSampleCollection s1 = StructuredSampleCollection(getTestSampleStructure(), getTestSampleData());
  
  std::stringstream sstream;
  
  writeArff(sstream, s1);
  
  std::cout << "STR: " << sstream.str() << std::endl;
  
  StructuredSampleCollection s2 = readArff(sstream);
  
  std::cout << s2;

  //Compare equality of the original and the read datasets
  if(s1 != s2){
    std::cout << "Arff test 1: FAILED" << std::endl;
    std::cout << "Original \"" << s1 << "\", new \"" << s2 << "\"" << std::endl;
  }
  else{
    std::cout << "Arff test 1: PASSED." << std::endl;
  }

  std::stringstream sstream2;
  
  writeArff(sstream2, s2);
  
  if(sstream.str() == sstream2.str()){
    std::cout << "Arff test 2: PASSED." << std::endl;
  }
  else{
    std::cout << "Arff test 2: Failed.  Sample collections are not invariant under multiple read/write cycles." << std::endl;
    std::cout << "Original \"" << sstream.str() << "\", new \"" << sstream2.str() << "\"" << std::endl;
  }
}

#define DE_TEST_SIZE 2048
#define DISTSIZE 4
void testDifferentialEntropy(){
  std::cout << "Testing Differential Entropy." << std::endl;
  
  fracfloat_t samples[DE_TEST_SIZE];
  
  std::default_random_engine generator;
  std::normal_distribution<fracfloat_t> distributions[DISTSIZE];
  
  distributions[0] = std::normal_distribution<fracfloat_t>(0.0,1.0);
  distributions[1] = std::normal_distribution<fracfloat_t>(0.1,4.0);
  distributions[2] = std::normal_distribution<fracfloat_t>(2.0,1.0);
  distributions[3] = std::normal_distribution<fracfloat_t>(5.0,8.0);
  
  
  for(unsigned i = 0; i < DE_TEST_SIZE; i++){
    samples[i] = distributions[i % DISTSIZE](generator);
  }
  
  Gaussian g = Gaussian(samples, DE_TEST_SIZE);
  MultiGaussian mg = MultiGaussian::fitGaussianKernel(samples, DE_TEST_SIZE, (unsigned)sqrt(DE_TEST_SIZE));
  
  fracfloat_t supportStart = min<fracfloat_t>(samples, DE_TEST_SIZE) * 2;
  fracfloat_t supportEnd = max<fracfloat_t>(samples, DE_TEST_SIZE) * 2;
  
  std::cout << "Gaussian: " << g << std::endl;
  std::cout << "Multi Gaussian: " << mg << std::endl;
  
  std::cout << "Integral gaussian approximation: " << g.integrate(supportStart, supportEnd, 1000) << std::endl;
  std::cout << "Integral multigaussian approximation: " << mg.integrate(supportStart, supportEnd, 1000) << std::endl;
  std::cout << "Integral 10: " << mg.approximateDifferentialEntropyFromIntegral(supportStart, supportEnd, 10) << std::endl;
  std::cout << "Integral 100: " << mg.approximateDifferentialEntropyFromIntegral(supportStart, supportEnd, 100) << std::endl;
  std::cout << "Integral 1000: " << mg.approximateDifferentialEntropyFromIntegral(supportStart, supportEnd, 1000) << std::endl;
  std::cout << "Distribution Trick:      " << mg.approximateDifferentialEntropyFromSamples(Array<fracfloat_t>(samples, DE_TEST_SIZE / 1)) << std::endl;
  std::cout << "Distribution Trick 1/2:  " << mg.approximateDifferentialEntropyFromSamples(Array<fracfloat_t>(samples, DE_TEST_SIZE / 2)) << std::endl;
  std::cout << "Distribution Trick 1/4:  " << mg.approximateDifferentialEntropyFromSamples(Array<fracfloat_t>(samples, DE_TEST_SIZE / 4)) << std::endl;
  std::cout << "Distribution Trick 1/8:  " << mg.approximateDifferentialEntropyFromSamples(Array<fracfloat_t>(samples, DE_TEST_SIZE / 8)) << std::endl;
  std::cout << "Distribution Trick 1/16: " << mg.approximateDifferentialEntropyFromSamples(Array<fracfloat_t>(samples, DE_TEST_SIZE / 16)) << std::endl;
  
}

void testUnknowns(){
  assert(isUnknownContVar(UNKNOWN_CONT));
  assert(isUnknownCatVar(UNKNOWN_CAT));
  
  assert(!isUnknownContVar((fracfloat_t)0));
  assert(!isUnknownCatVar(0));
}

int main(){
  
  testUnknowns();
  
  //testGaussianNumerics();
  
  validateTestData();
  
  testWafflesLoading();
  
  testWafflesDT();
  
  //testPredictor(getTestPredictorWaffles());
  
  testNormalizedSurprisal();

  //testFrac(getTestPredictorWaffles());
  
  testArff();

  //testPredictor(getTestPredictorLibSvm());
  
  //testFrac(getTestPredictorLibSvm());

  testDifferentialEntropy();
}
