#ifndef WAFFLES_H
#define	WAFFLES_H

#include <exception>
#include <iostream>
#include <vector>

//We have the dire misfortune of headers declaring unwanted misfortunes.  This was an attempt to stop them from doing so.
//#define throw (void)
//#define Ex(a) (void)

//#include <GClasses/GApp.h>
//#include <GClasses/GError.h>
#include <GClasses/GLearner.h>
#include <GClasses/GDecisionTree.h>
#include <GClasses/GNeuralNet.h>
#include <GClasses/GDom.h>

#include <GClasses/GActivation.h>
#include <GClasses/GKNN.h>
#include <GClasses/GNaiveBayes.h>
#include <GClasses/GEnsemble.h>

#include "mltypes.hpp"
#include <iostream>

//Helper functions, primarily to build and load into waffles types

//Create matrix schemata
GClasses::GMatrix generateContinuousFeatureSchema(SampleStructure& sampleStructure, unsigned featureIndex);
GClasses::GMatrix generateCategoricalFeatureSchema(SampleStructure& sampleStructure, unsigned featureIndex);
GClasses::GMatrix generateBinaryFeatureSchema(SampleStructure& sampleStructure, unsigned featureIndex);

GClasses::GMatrix generateContinuousLabelSchema(SampleStructure& sampleStructure, unsigned featureIndex);
GClasses::GMatrix generateCategoricalLabelSchema(SampleStructure& sampleStructure, unsigned featureIndex);
GClasses::GMatrix generateBinaryLabelSchema(SampleStructure& sampleStructure, unsigned featureIndex);

//Loads a sample to a vector, excluding the label feature.
void loadSampleToRegressorVector(double* featureRow, SampleStructure sampleStructure, Sample* sample, unsigned featureIndex);
void loadSampleToClassifierVector(double* featureRow, SampleStructure sampleStructure, Sample* sample, unsigned featureIndex);
void loadSampleToBinaryClassifierVector(double* featureRow, SampleStructure sampleStructure, Sample* sample, unsigned featureIndex);

//Given an array of training data, split it into features and labels.
void loadRegressorTrainingData(GClasses::GMatrix& featureMatrix, GClasses::GMatrix& labelMatrix, SampleStructure sampleStructure, unsigned featureIndex, Array<Sample> training);
void loadClassifierTrainingData(GClasses::GMatrix& featureMatrix, GClasses::GMatrix& labelMatrix, SampleStructure& sampleStructure, unsigned featureIndex, Array<Sample> training);
void loadBinaryClassifierTrainingData(GClasses::GMatrix& featureMatrix, GClasses::GMatrix& labelMatrix, SampleStructure& sampleStructure, unsigned featureIndex, Array<Sample> training);


/////////////
//Regressors:

class WafflesDecisionTreeRegressor : public Regressor{
  private:
  SampleStructure sampleStructure;
  unsigned featureIndex;
  GClasses::GDecisionTree model;
  //bool useRandomTree;
  
  public:
  WafflesDecisionTreeRegressor(){}//For array construction.
  WafflesDecisionTreeRegressor(SampleStructure s, unsigned f) : sampleStructure(s), featureIndex(f) { }
  WafflesDecisionTreeRegressor(SampleStructure s, unsigned f, bool random) : sampleStructure(s), featureIndex(f) /*, useRandomTree(random) */ { if(random) model.useRandomDivisions(1); }
  
  void train(Array<Sample>);
  contvar_t predict(Sample);
  
  //NOTE: Has predictDistribution.
  
  void writeToStream(std::ostream& o) const{
    o << "{Waffles Decision Tree Regressor " << "{" << " feature " << featureIndex << "} { [|";

    GClasses::GDecisionTree* modelPtr = ((GClasses::GDecisionTree*)((void*)&model));

    modelPtr->print(o);
    
    o << "|] } }";
  }
  
  void freeData(){
    model.clear();
  }
};

//////////////
//Classifiers:

class WafflesDecisionTreeClassifier : public ProbabilisticClassifier{
  private:
  public:
  SampleStructure sampleStructure;
  unsigned featureIndex;
  GClasses::GDecisionTree model;
  //bool useRandomTree;
  
  public:
  WafflesDecisionTreeClassifier(){} //For array construction.
  WafflesDecisionTreeClassifier(SampleStructure s, unsigned f) : sampleStructure(s), featureIndex(f) { }
  WafflesDecisionTreeClassifier(SampleStructure s, unsigned f, bool random) : sampleStructure(s), featureIndex(f) {
    if(random > 0){
      model.useRandomDivisions(1);
    }
  }
  void train(Array<Sample>);
  catvar_t predict(Sample);
  unsigned predictDistributionClassCount();
  void predictDistribution(Sample s, fracfloat_t* out);
  
  //NOTE: Has predictDistribution.
  
  void writeToStream(std::ostream& o) const{
    o << "{Waffles Decision Tree Classifier " << "{" << "feature: " << "\"" << sampleStructure.catvarName(featureIndex) << "\"" << "} { [|";

    /*
     void GClasses::GDecisionTree::print 	( 	std::ostream &  	stream,
		GArffRelation *  	pFeatureRel = NULL,
		GArffRelation *  	pLabelRel = NULL 
	) 		

Prints an ascii representation of the decision tree to the specified stream. pRelation is an optional relation that can be supplied in order to provide better meta-data to make the print-out richer.
*/
    
    //Waffles does not declare const functions consistently.  Get a non const version of the model, without copy semantics.
    
    //Just pretend you don't see this.  Move along, there's nothing to see here.
    GClasses::GDecisionTree* modelPtr = ((GClasses::GDecisionTree*)((void*)&model));
    
    #define FANCY_MODEL_PRINT
    #ifdef  FANCY_MODEL_PRINT
    GClasses::GArffRelation features, label;
    
    //Not used in printing
    //features.setName("Features");
    //label.setName("Label");
        

    //Push contvars
    std::vector<const char*> emptyVec;
    for(unsigned i = 0; i < sampleStructure.contvarCount; i++){
      features.addAttribute(sampleStructure.contvarName(i).c_str(), 0, &emptyVec);
    }
    
    //Push catvars
    std::vector<const char*> catvarVec[sampleStructure.catvarCount];
    for(unsigned i = 0; i < sampleStructure.catvarCount; i++){
      catvarVec[i].reserve(sampleStructure.catvarSizes[i]);
      for(unsigned j = 0; j < sampleStructure.catvarSizes[i]; j++){
        catvarVec[i].push_back(sampleStructure.catvarCatNameLookup(i, j)->c_str()); //Optimization: Can this be done without copying?
      }
      if(i == featureIndex) continue;
      features.addAttribute(sampleStructure.catvarName(i).c_str(), sampleStructure.catvarSizes[i], catvarVec + i);
    }
    
    //Push binvars
    std::vector<const char*> binvarVec[sampleStructure.binvarCount];
    for(unsigned i = 0; i < sampleStructure.binvarCount; i++){
      binvarVec[i].reserve(2);
      for(unsigned i = 0; i < 2; i++){
        binvarVec[i].push_back(sampleStructure.binvarCatNameLookup(i, 0)->c_str());
      }
      
      features.addAttribute(sampleStructure.binvarName(i).c_str(), 2, binvarVec + i);
    }

    
    //Label
    label.addAttribute(sampleStructure.catvarName(featureIndex).c_str(), sampleStructure.catvarSizes[featureIndex], catvarVec + featureIndex);
    
    
    //Now print the model
    modelPtr->print(o, &features, &label);

    #else
    modelPtr->print(o);
    #endif
    
    //Architecture: consider using metadata info for a better printout.
    
    o << "|] } }";
#if 0
    //Version using serialization:
    //These are actually even worse...
    GClasses::GDom doc;
    GClasses::GDomNode* serialized = modelPtr->serialize(&doc);
    doc.setRoot(serialized);
    
    
    o << "|] }";
    
    o << "{ [|";
    
    doc.writeJsonPretty(o);
    
    o << "|] }";
    
    o << "{ [|";
    
    doc.writeJsonCpp(o);
    
    o << "|] }";
    
    o << "{ [|";
    
    doc.writeJson(o);
    
    o << "|] }";
    
    o << "{ [|";
    
    doc.writeXml(o);
    
    o << "]| }";
    
    o << "}";
#endif
  }
  
  void freeMemory(){
    model.clear();
  }
};

enum WafflesClassifierTy {NN2, NN3, NaiveBayes, MeanMarginsTree, RandomForest, Wag};

extern std::string classifierTyNames[];

class WafflesGenericClassifier : public ProbabilisticClassifier{
  private:
  SampleStructure sampleStructure;
  unsigned featureIndex;
  
  WafflesClassifierTy ty;
  
  GClasses::GSupervisedLearner* classifier;
  GClasses::GAutoFilter* autoFilter;
  
  bool useAutoTune;
  
  public:
  WafflesGenericClassifier(){}
  WafflesGenericClassifier(SampleStructure ss, unsigned f, WafflesClassifierTy ty, GClasses::GSupervisedLearner* classifier, GClasses::GAutoFilter* autoFilter, bool autoTune = true) : sampleStructure(ss), featureIndex(f), ty(ty), classifier(classifier), autoFilter(autoFilter), useAutoTune(autoTune) { }
  WafflesGenericClassifier(SampleStructure s, unsigned f, WafflesClassifierTy ty, bool autoTune = true) : sampleStructure(s), featureIndex(f), ty(ty), useAutoTune(autoTune){
    
    switch(ty){
      case NN2:
      case NN3:
      {
    
        //Neural Net
    
        unsigned catvarCount = s.catvarSizes[f];
        
        GClasses::GNeuralNet* network = new GClasses::GNeuralNet();
        network->addLayer(new GClasses::GLayerClassic(FLEXIBLE_SIZE, FLEXIBLE_SIZE));
        if(ty == NN3){
          //Architecture: Options on the size of this layer.
          unsigned midLayerSize = ((s.featureCount() - 1) + catvarCount) / 2;
          network->addLayer(new GClasses::GLayerClassic(midLayerSize, FLEXIBLE_SIZE));
        }
        else{
          network->addLayer(new GClasses::GLayerClassic(catvarCount, FLEXIBLE_SIZE));
        }
        network->setLearningRate(0.1);
        network->setMomentum(0.1);
        
        classifier = network;
        
        break;
      }
      case NaiveBayes:    
      {
        classifier = new GClasses::GNaiveBayes();
        break;
      }
      case MeanMarginsTree:
      {
        classifier = new GClasses::GMeanMarginsTree();
        break;
      }
      case RandomForest:
      {
        classifier = new GClasses::GRandomForest(16);
        break;
      }
      case Wag:
      {
        classifier = new GClasses::GWag(4);
        break;
      }
      default:
      {
        assert(0);
      }
    }
    
    autoFilter = new GClasses::GAutoFilter(classifier);
  }
  
  static WafflesGenericClassifier* makeRandomForest(SampleStructure ss, unsigned f, unsigned size){
    GClasses::GSupervisedLearner* c = new GClasses::GRandomForest(size);
    GClasses::GAutoFilter* fltr = new GClasses::GAutoFilter(c);
    return new WafflesGenericClassifier(ss, f, RandomForest, c, fltr);
  }
  
  void train(Array<Sample>);
  catvar_t predict(Sample);
  
  unsigned predictDistributionClassCount();
  void predictDistribution(Sample, fracfloat_t*);
  
  void writeToStream(std::ostream& o) const{
    o << "{Waffles Classifier: " << "{" << "feature: " << featureIndex << ", classifier type: " << classifierTyNames[ty] << "}}";
  }
  
  void freeMemory(){
    classifier->clear();
    autoFilter->clear();
    delete autoFilter;
    //delete classifier; //Gets deleted by the autofilter.
  }
};


/////////////////////
//Binary Classifiers:

class WafflesDecisionTreeBinaryClassifier : public ProbabilisticBinaryClassifier{
  private:
  SampleStructure sampleStructure;
  unsigned featureIndex;
  GClasses::GDecisionTree model;
  //bool useRandomTree;
  
  public:
  WafflesDecisionTreeBinaryClassifier(){} //For array construction.
  WafflesDecisionTreeBinaryClassifier(SampleStructure s, unsigned f) : sampleStructure(s), featureIndex(f) { }
  WafflesDecisionTreeBinaryClassifier(SampleStructure s, unsigned f, bool random) : sampleStructure(s), featureIndex(f) {
    if(random > 0){
      model.useRandomDivisions(1);
    }
  }
  void train(Array<Sample>);
  binvar_t predict(Sample);
  
  unsigned predictDistributionClassCount();
  fracfloat_t predictDistribution(Sample s);
  
  
  void writeToStream(std::ostream& o) const{
    o << "{Waffles Decision Tree Binary Classifier " << "{" << " feature " << featureIndex << "} { [|";
    
    //Waffles does not declare const functions consistently.  Get a non const version of the model, without copy semantics.
    
    //Just pretend you don't see this.  Move along, there's nothing to see here.
    GClasses::GDecisionTree* modelPtr = ((GClasses::GDecisionTree*)((void*)&model));
    
    modelPtr->print(o);
    
    o << "|] } }";
  }
  
  void freeMemory(){
    model.clear();
  }
};

#endif