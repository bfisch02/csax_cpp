#include "waffles.hpp"

#include <vector>

#include <GClasses/GDistribution.h>

/////////////////////////////////////
//WAFFLES DATATYPE LOADER FUNCTIONS//
/////////////////////////////////////

//Waffles uses GMatrix types to represent both the Sample and SampleType objects.  Because we can't look at feature X in a predictor for feature X, we must strip it out in the conversion, which makes things arduous.
//Additionally, Waffles uses the GMatrix type to represent labels, which are used by the classifier to determine what it is supposed to be learning.

//This seems like a large amount of code, but it is linear with the number of classifiers + the number of feature types.  The alternative would be to hardcode all these short functions into a function for each classifier type, but this would require code that is linear with the product of the previous two terms.

///////////////////////////
//FEATURE SCHEMA GENERATORS

//Functions for creating the schema, which is a GMatrix containing the information of a SampleType, minus the feature to be learned.
static void pushAllContinuousFeatureSchema(std::vector<size_t>& featureSchema, SampleStructure& sampleStructure){
  for(unsigned i = 0; i < sampleStructure.contvarCount; i++){
    featureSchema.push_back(0); //0 for cont vals.
  }
}
//#include <iostream>
static void pushAllExceptContinuousFeatureSchema(std::vector<size_t>& featureSchema, SampleStructure& sampleStructure, unsigned except){
  for(unsigned i = 1; i < sampleStructure.contvarCount; i++){ //Just push one fewer, exactly what is being pushed doesn't really matter.
    featureSchema.push_back(0);
  }
}
static void pushAllCategoricalFeatureSchema(std::vector<size_t>& featureSchema, SampleStructure& sampleStructure){
  for(unsigned i = 0; i < sampleStructure.catvarCount; i++){
    featureSchema.push_back(sampleStructure.catvarSizes[i]);
  }
}
static void pushAllExceptCategoricalFeatureSchema(std::vector<size_t>& featureSchema, SampleStructure& sampleStructure, unsigned except){
  for(unsigned i = 0; i < sampleStructure.catvarCount; i++){
    if(i == except) continue; //Optimization: split this loop
    featureSchema.push_back(sampleStructure.catvarSizes[i]);
  }
}

static void pushAllBinaryFeatureSchema(std::vector<size_t>& featureSchema, SampleStructure& sampleStructure){
  for(unsigned i = 0; i < sampleStructure.binvarCount; i++){
    featureSchema.push_back(2);
  }
}
static void pushAllExceptBinaryFeatureSchema(std::vector<size_t>& featureSchema, SampleStructure& sampleStructure, unsigned except){
  for(unsigned i = 1; i < sampleStructure.binvarCount; i++){ //Just push one fewer, exactly what is being pushed doesn't really matter.
    featureSchema.push_back(2);
  }
}

GClasses::GMatrix generateContinuousFeatureSchema(SampleStructure& sampleStructure, unsigned featureIndex){
  std::vector<size_t> featureSchema;
  featureSchema.reserve(sampleStructure.contvarCount + sampleStructure.catvarCount + sampleStructure.binvarCount - 1); //Reserve space to prevent any resizing.
  pushAllExceptContinuousFeatureSchema(featureSchema, sampleStructure, featureIndex);
  pushAllCategoricalFeatureSchema(featureSchema, sampleStructure);
  pushAllBinaryFeatureSchema(featureSchema, sampleStructure);
  return featureSchema;
}

GClasses::GMatrix generateCategoricalFeatureSchema(SampleStructure& sampleStructure, unsigned featureIndex){
  std::vector<size_t> featureSchema;
  featureSchema.reserve(sampleStructure.contvarCount + sampleStructure.catvarCount + sampleStructure.binvarCount - 1); //Reserve space to prevent any resizing.
  pushAllContinuousFeatureSchema(featureSchema, sampleStructure);
  pushAllExceptCategoricalFeatureSchema(featureSchema, sampleStructure, featureIndex);
  pushAllBinaryFeatureSchema(featureSchema, sampleStructure);
  return featureSchema;
}

GClasses::GMatrix generateBinaryFeatureSchema(SampleStructure& sampleStructure, unsigned featureIndex){
  std::vector<size_t> featureSchema;
  featureSchema.reserve(sampleStructure.contvarCount + sampleStructure.catvarCount + sampleStructure.binvarCount - 1); //Reserve space to prevent any resizing.
  pushAllContinuousFeatureSchema(featureSchema, sampleStructure);
  pushAllCategoricalFeatureSchema(featureSchema, sampleStructure);
  pushAllExceptBinaryFeatureSchema(featureSchema, sampleStructure, featureIndex);
  return featureSchema;
}

/////////////////////////
//LABEL SCHEMA GENERATORS

//These are simple: each label contains only the feature to be predicted.

GClasses::GMatrix generateContinuousLabelSchema(SampleStructure& sampleStructure, unsigned featureIndex){
  std::vector<size_t> labelSchema(1);
  labelSchema[0] = 0;
  
  GClasses::GMatrix labelSchemaMatrix = GClasses::GMatrix(labelSchema);
  return labelSchemaMatrix;
}

GClasses::GMatrix generateCategoricalLabelSchema(SampleStructure& sampleStructure, unsigned featureIndex){
  std::vector<size_t> labelSchema(1);
  labelSchema[0] = sampleStructure.catvarSizes[featureIndex];
  
  GClasses::GMatrix labelSchemaMatrix = GClasses::GMatrix(labelSchema);
  return labelSchemaMatrix;
}

GClasses::GMatrix generateBinaryLabelSchema(SampleStructure& sampleStructure, unsigned featureIndex){
  std::vector<size_t> labelSchema(1);
  labelSchema[0] = 2;
  
  GClasses::GMatrix labelSchemaMatrix = GClasses::GMatrix(labelSchema);
  return labelSchemaMatrix;
}

//////////////
//DATA LOADERS

static double unknownCorrectedContinuousFeature(SampleStructure& ss, Sample& s, unsigned index){
  double val = s.contvarUnknown(index) ? ss.contvarMeans[index] : s.contvars[index];
  return val;
}

static double unknownCorrectedCategoricalFeature(SampleStructure& ss, Sample& s, unsigned index){
  double val = s.catvarUnknown(index) ? -1 : s.catvars[index];
  assert(val < ss.catvarSizes[index]);
  return val;
}

static double unknownCorrectedBinaryFeature(SampleStructure& ss, Sample& s, unsigned index){
  double val = s.binvarUnknown(index) ? -1 : s.binvars[index];
  assert(val == 0 || val == 1 || val == -1);
  return val;
}

//Functions to load data into feature matrices
static void loadAllContinuousFeatures(double* cpyPtr, SampleStructure& sampleStructure, Sample& sample){
  for(unsigned i = 0; i < sampleStructure.contvarCount; i++){
    cpyPtr[i] = unknownCorrectedContinuousFeature(sampleStructure, sample, i);
  }
}
static void loadAllExceptContinuousFeatures(double* cpyPtr, SampleStructure& sampleStructure, Sample& sample, unsigned except){
  assert(except < sampleStructure.contvarCount);
  for(unsigned i = 0; i < except; i++){ //Just push one fewer, exactly what is being pushed doesn't really matter.
    cpyPtr[i] = unknownCorrectedContinuousFeature(sampleStructure, sample, i);
  }
  for(unsigned i = except + 1; i < sampleStructure.contvarCount; i++){
    cpyPtr[i - 1] = unknownCorrectedContinuousFeature(sampleStructure, sample, i);
  }
}
static void loadAllCategoricalFeatures(double* cpyPtr, SampleStructure& sampleStructure, Sample& sample){
  for(unsigned i = 0; i < sampleStructure.catvarCount; i++){
    cpyPtr[i] = unknownCorrectedCategoricalFeature(sampleStructure, sample, i);
  }
}
static void loadAllExceptCategoricalFeatures(double* cpyPtr, SampleStructure& sampleStructure, Sample& sample, unsigned except){
  assert(except < sampleStructure.catvarCount);
  for(unsigned i = 0; i < except; i++){
    cpyPtr[i] = unknownCorrectedCategoricalFeature(sampleStructure, sample, i);
  }
  for(unsigned i = except + 1; i < sampleStructure.catvarCount; i++){
    cpyPtr[i - 1] = unknownCorrectedCategoricalFeature(sampleStructure, sample, i);
  }
}
static void loadAllBinaryFeatures(double* cpyPtr, SampleStructure& sampleStructure, Sample& sample){
  for(unsigned i = 0; i < sampleStructure.binvarCount; i++){
    cpyPtr[i] = unknownCorrectedBinaryFeature(sampleStructure, sample, i);
  }
}
static void loadAllExceptBinaryFeatures(double* cpyPtr, SampleStructure& sampleStructure, Sample& sample, unsigned except){
  assert(except < sampleStructure.binvarCount);
  for(unsigned i = 0; i < except; i++){ //Just push one fewer, exactly what is being pushed doesn't really matter.
    cpyPtr[i] = unknownCorrectedBinaryFeature(sampleStructure, sample, i);
  }
  for(unsigned i = except + 1; i < sampleStructure.contvarCount; i++){
    cpyPtr[i - 1] = unknownCorrectedBinaryFeature(sampleStructure, sample, i);
  }
}

//Functions to load samples into vectors, represented as the raw double* used by waffles, dropping the necessary feature.
//Note that the pointer must be added before loading.  Sizes of previous types are added, and 1 is subtracted if one less was added in a previous category.
static void loadSampleToRegressorVector(double* cpyPtr, SampleStructure& sampleStructure, Sample& sample, unsigned featureIndex){
  loadAllExceptContinuousFeatures(cpyPtr, sampleStructure, sample, featureIndex);
  loadAllCategoricalFeatures(cpyPtr + sampleStructure.contvarCount - 1, sampleStructure, sample);
  loadAllBinaryFeatures(cpyPtr + sampleStructure.contvarCount + sampleStructure.catvarCount - 1, sampleStructure, sample);
}

static void loadSampleToClassifierVector(double* cpyPtr, SampleStructure& sampleStructure, Sample& sample, unsigned featureIndex){
  loadAllContinuousFeatures(cpyPtr, sampleStructure, sample);
  loadAllExceptCategoricalFeatures(cpyPtr + sampleStructure.contvarCount, sampleStructure, sample, featureIndex);
  loadAllBinaryFeatures(cpyPtr + sampleStructure.contvarCount + sampleStructure.catvarCount - 1, sampleStructure, sample);
}

static void loadSampleToBinaryClassifierVector(double* cpyPtr, SampleStructure& sampleStructure, Sample& sample, unsigned featureIndex){
  loadAllContinuousFeatures(cpyPtr, sampleStructure, sample);
  loadAllCategoricalFeatures(cpyPtr + sampleStructure.contvarCount, sampleStructure, sample);
  loadAllExceptBinaryFeatures(cpyPtr + sampleStructure.contvarCount + sampleStructure.catvarCount, sampleStructure, sample, featureIndex);
}

//These functions load a training set into the GMatrices.
void loadRegressorTrainingData(GClasses::GMatrix& featureMatrix, GClasses::GMatrix& labelMatrix, SampleStructure sampleStructure, unsigned featureIndex, Array<Sample> training){
  for(unsigned ti = 0; ti < training.length; ti++){
    assert(!training[ti].contvarUnknown(featureIndex)); //We should never be predicting an unknown value in FRaC.
    double* cpyPtr = featureMatrix.newRow();
    loadSampleToRegressorVector(cpyPtr, sampleStructure, training[ti], featureIndex);
    double* labelRow = labelMatrix.newRow();
    labelRow[0] = training[ti].contvars[featureIndex];
  }
}

void loadClassifierTrainingData(GClasses::GMatrix& featureMatrix, GClasses::GMatrix& labelMatrix, SampleStructure& sampleStructure, unsigned featureIndex, Array<Sample> training){
  for(unsigned ti = 0; ti < training.length; ti++){
    assert(!training[ti].catvarUnknown(featureIndex)); //We should never be predicting an unknown value in FRaC.
    double* cpyPtr = featureMatrix.newRow();
    loadSampleToClassifierVector(cpyPtr, sampleStructure, training[ti], featureIndex);
    double* labelRow = labelMatrix.newRow();
    labelRow[0] = training[ti].catvars[featureIndex];
  }
}

void loadBinaryClassifierTrainingData(GClasses::GMatrix& featureMatrix, GClasses::GMatrix& labelMatrix, SampleStructure& sampleStructure, unsigned featureIndex, Array<Sample> training){
  for(unsigned ti = 0; ti < training.length; ti++){
    assert(!training[ti].binvarUnknown(featureIndex)); //We should never be predicting an unknown value in FRaC.
    double* cpyPtr = featureMatrix.newRow();
    loadSampleToBinaryClassifierVector(cpyPtr, sampleStructure, training[ti], featureIndex);
    double* labelRow = labelMatrix.newRow();
    labelRow[0] = training[ti].binvars[featureIndex];
  }
}


/////////////////////
//Regressor functions

void WafflesDecisionTreeRegressor::train(Array<Sample> training){
  //Create schema for feature and label matrices.
  GClasses::GMatrix featureMatrix = generateContinuousFeatureSchema(sampleStructure, featureIndex);
  GClasses::GMatrix labelMatrix   = generateContinuousLabelSchema(sampleStructure, featureIndex);
  
  //Populate the feature and label matrices with the dataset.
  loadRegressorTrainingData(featureMatrix, labelMatrix, sampleStructure, featureIndex, training);

  //Train the model.
  model.autoTune(featureMatrix, labelMatrix);
  model.train(featureMatrix, labelMatrix);
}

contvar_t WafflesDecisionTreeRegressor::predict(Sample s){
  
  //Stack allocate data.  
  //Note: Compiler support for dynamic stack allocation is spotty.
  double inputVector[sampleStructure.featureCount() - 1]; //an array of features is the input
  loadSampleToRegressorVector(inputVector, sampleStructure, s, featureIndex);
  double prediction[1]; //1 feature is being predicted.
  
  model.predict(inputVector, prediction);

  return *prediction;
}

//////////////////////
//Classifier Functions

///////////////
//DECISION TREE

void WafflesDecisionTreeClassifier::train(Array<Sample> training){
  //Create schema for feature and label matrices.
  GClasses::GMatrix featureMatrix = generateCategoricalFeatureSchema(sampleStructure, featureIndex);
  GClasses::GMatrix labelMatrix   = generateCategoricalLabelSchema(sampleStructure, featureIndex);
  
  //Populate the feature and label matrices with the dataset.
  loadClassifierTrainingData(featureMatrix, labelMatrix, sampleStructure, featureIndex, training);

  //Train the model.
  
  //Architecture: Option to auto tune option.
  //model.autoTune(featureMatrix, labelMatrix);
  model.train(featureMatrix, labelMatrix);
}

catvar_t WafflesDecisionTreeClassifier::predict(Sample s){
  
  //Stack allocate data.  
  //Note: Compiler support for dynamic stack allocation is spotty.
  double inputVector[sampleStructure.featureCount() - 1]; //an array of features is the input
  loadSampleToClassifierVector(inputVector, sampleStructure, s, featureIndex);
  double prediction[1]; //1 feature is being predicted.
  
  model.predict(inputVector, prediction);

  return (catvar_t) *prediction;
}

unsigned WafflesDecisionTreeClassifier::predictDistributionClassCount(){
  return sampleStructure.catvarSizes[featureIndex];
}

void WafflesDecisionTreeClassifier::predictDistribution(Sample s, fracfloat_t* out){
  double inputVector[sampleStructure.featureCount() - 1]; //an array of features is the input
  loadSampleToClassifierVector(inputVector, sampleStructure, s, featureIndex);
  
  GClasses::GPrediction prediction;
  
  model.predictDistribution(inputVector, &prediction);

  GClasses::GCategoricalDistribution* distribution = prediction.asCategorical();
  
  for(unsigned i = 0; i < sampleStructure.catvarSizes[featureIndex]; i++){
    out[i] = (fracfloat_t) distribution->likelihood(i);
  }
  
  assert(epsilonCompare<fracfloat_t>(1, sumTerms<fracfloat_t>(out, sampleStructure.catvarSizes[featureIndex])));
  
  //Memory: free distribution.
}

////////////////////////////
//GENERIC WAFFLES CLASSIFIER

std::string classifierTyNames[] = {"2 Layer Neural Net", "3 Layer Neural Net", "Naive Bayes Classifier", "Mean Margins Tree", "Random Forest", "Wag"};

void WafflesGenericClassifier::train(Array<Sample> training){
  //Create schema for feature and label matrices.
  GClasses::GMatrix featureMatrix = generateCategoricalFeatureSchema(sampleStructure, featureIndex);
  GClasses::GMatrix labelMatrix   = generateCategoricalLabelSchema(sampleStructure, featureIndex);
  
  //Populate the feature and label matrices with the dataset.
  loadClassifierTrainingData(featureMatrix, labelMatrix, sampleStructure, featureIndex, training);

  //Tune the model with CV
  //autoFilter->autoTune(featureMatrix, labelMatrix);
  //TODO need to enable this functionality in Waffles.
  
  //Train the model.
  autoFilter->train(featureMatrix, labelMatrix);
}

catvar_t WafflesGenericClassifier::predict(Sample s){
  
  //Stack allocate data.  
  //Note: Compiler support for dynamic stack allocation is spotty.
  double inputVector[sampleStructure.featureCount() - 1]; //an array of features is the input
  loadSampleToClassifierVector(inputVector, sampleStructure, s, featureIndex);
  double prediction[1]; //1 feature is being predicted.
  
  autoFilter->predict(inputVector, prediction);

  return *prediction;
}

unsigned WafflesGenericClassifier::predictDistributionClassCount(){
  return sampleStructure.catvarSizes[featureIndex];
}

void WafflesGenericClassifier::predictDistribution(Sample s, fracfloat_t* out){
  
  //Stack allocate data.  
  //Note: Compiler support for dynamic stack allocation is spotty.
  double inputVector[sampleStructure.featureCount() - 1]; //an array of features is the input
  loadSampleToClassifierVector(inputVector, sampleStructure, s, featureIndex);
  
  GClasses::GPrediction prediction;
  
  autoFilter->predictDistribution(inputVector, &prediction);

  GClasses::GCategoricalDistribution* distribution = prediction.asCategorical();
    
  for(unsigned i = 0; i < sampleStructure.catvarSizes[featureIndex]; i++){
    out[i] = (fracfloat_t) distribution->likelihood(i);
  }
  
  assert(epsilonCompare<fracfloat_t>(1, sumTerms<fracfloat_t>(out, sampleStructure.catvarSizes[featureIndex])));
    
  //Memory: free distribution.
}

/////////////////////////////
//Binary Classifier functions


void WafflesDecisionTreeBinaryClassifier::train(Array<Sample> training){
  //Create schema for feature and label matrices.
  GClasses::GMatrix featureMatrix = generateBinaryFeatureSchema(sampleStructure, featureIndex);
  GClasses::GMatrix labelMatrix   = generateBinaryLabelSchema(sampleStructure, featureIndex);
  
  //Populate the feature and label matrices with the dataset.
  loadBinaryClassifierTrainingData(featureMatrix, labelMatrix, sampleStructure, featureIndex, training);

  //Train the model.
  
  model.autoTune(featureMatrix, labelMatrix);
  model.train(featureMatrix, labelMatrix);
}

binvar_t WafflesDecisionTreeBinaryClassifier::predict(Sample s){
  
  //Stack allocate data.  
  //Note: Compiler support for dynamic stack allocation is spotty.
  double inputVector[sampleStructure.featureCount() - 1]; //an array of features is the input
  loadSampleToBinaryClassifierVector(inputVector, sampleStructure, s, featureIndex);
  double prediction[1]; //1 feature is being predicted.
  
  model.predict(inputVector, prediction);

  return (binvar_t) *prediction;
}

fracfloat_t WafflesDecisionTreeBinaryClassifier::predictDistribution(Sample s){
  double inputVector[sampleStructure.featureCount() - 1]; //an array of features is the input
  loadSampleToBinaryClassifierVector(inputVector, sampleStructure, s, featureIndex);
  
  GClasses::GPrediction prediction;
  
  model.predictDistribution(inputVector, &prediction);

  GClasses::GCategoricalDistribution* distribution = prediction.asCategorical();
  
  //Optimization: could we access an individual element of the dist without loading the whole thing?
    
  fracfloat_t retval = (fracfloat_t) distribution->likelihood(0);
  
  assert(retval <= 1);
  assert(epsilonCompare(1.0, distribution->likelihood(0) + distribution->likelihood(1)));
  
  //Memory: free distribution.
  //delete [] distribution;
  
  return retval;
}
