#include <vector>
#include <algorithm>

#include "vectormath.hpp"
#include "errormodelcollection.hpp"

//Array indexing helper functions

static unsigned foldStart(unsigned fold, unsigned folds, unsigned size){
  return size * fold / folds;
}

//For indexing into the predicted results storage arrays.

//No fold information
static unsigned dataIndex(unsigned predictor, unsigned feature, unsigned shuffle, unsigned sample, unsigned featureCount, unsigned shuffleCount, unsigned sampleCount){
  assert(feature < featureCount);
  assert(shuffle < shuffleCount);
  assert(sample < sampleCount);
  return predictor * featureCount * shuffleCount * sampleCount + feature * shuffleCount * sampleCount + shuffle * sampleCount + sample;
}

//Resolves index with fold information
static unsigned dataIndexWithFold(unsigned predictor, unsigned feature, unsigned shuffle, unsigned fold, unsigned sample, unsigned featureCount, unsigned shuffleCount, unsigned sampleCount, unsigned foldCount){
  return dataIndex(predictor, feature, shuffle, foldStart(fold, foldCount, sampleCount) + sample, featureCount, shuffleCount, sampleCount);
}

void ErrorModelCollection::buildErrorModels(SampleStructure sampleStructure, ErrorModelCreators emc, Array<Sample> truth, Array<Sample> predictions){
  assert(truth.length == predictions.length);
  
  //Allocate memory for the error models.

  continuousErrorModels = new ContinuousErrorModel*[sampleStructure.contvarCount];
  categoricalErrorModels = new CategoricalErrorModel*[sampleStructure.catvarCount];
  binaryErrorModels = new BinaryErrorModel*[sampleStructure.binvarCount];

  /*
  NOTE: This function could be more memory efficient by only holding predictions for one variable at a time.
   */

  //Optimization use this memory for the catvars and binvars too.

  Array<fracfloat_t> actualContvars = Array<fracfloat_t>(truth.length);
  Array<fracfloat_t> predictedContvars = Array<fracfloat_t>(truth.length);
  
  for(unsigned i = 0; i < sampleStructure.contvarCount; i++){
    //Collect the ith contvar from the original data and the predicted data.  (Essentially this is a transpose with a zip, one row at a time).
    truth.mapTo<fracfloat_t, unsigned>([](Sample s, unsigned i){return s.contvars[i];}, i, actualContvars);
    predictions. mapTo<fracfloat_t, unsigned>([](Sample s, unsigned i){return s.contvars[i];}, i, predictedContvars);

    //Use them to build error models.
    continuousErrorModels[i] = emc.rc(&sampleStructure, i, actualContvars.data, predictedContvars.data, truth.length);
  }

  actualContvars.freeMemory();
  predictedContvars.freeMemory();
  
  Array<catvar_t> actualCatvars = Array<catvar_t>(truth.length);
  Array<catvar_t> predictedCatvars = Array<catvar_t>(truth.length);

  for(unsigned i = 0; i < sampleStructure.catvarCount; i++){
    //Collect the ith contvar from the original data and the predicted data.  (Essentially this is a transpose with a zip, one row at a time).
    truth.mapTo<catvar_t, unsigned> ([](Sample s, unsigned i){return s.catvars[i];}, i, actualCatvars);
    predictions.mapTo<catvar_t, unsigned> ([](Sample s, unsigned i){return s.catvars[i];}, i, predictedCatvars);

    //Use them to build error models.
    categoricalErrorModels[i] = emc.cc(&sampleStructure, i, actualCatvars.data, predictedCatvars.data, truth.length);
  }

  actualCatvars.freeMemory();
  predictedCatvars.freeMemory();
  

  Array<binvar_t> actualBinvars = Array<binvar_t>(truth.length);
  Array<binvar_t> predictedBinvars = Array<binvar_t>(truth.length);

  for(unsigned i = 0; i < sampleStructure.binvarCount; i++){
    //Collect the ith contvar from the original data and the predicted data.  (Essentially this is a transpose with a zip, one row at a time).
    truth.mapTo<binvar_t, unsigned> ([](Sample s, unsigned i){return s.getBinvar(i);}, i, actualBinvars);
    predictions.mapTo<binvar_t, unsigned> ([](Sample s, unsigned i){return s.getBinvar(i);}, i, predictedBinvars);

    //Use them to build error models.
    binaryErrorModels[i] = emc.bcc(&sampleStructure, i, actualBinvars.data, predictedBinvars.data, truth.length);
  }

  actualBinvars.freeMemory();
  predictedBinvars.freeMemory();
  
}

void ErrorModelCollection::buildErrorModels(SampleStructure sampleStructure, ErrorModelCreators emc, contvar_t* trueContvars, contvar_t* predContvars, catvar_t* trueCatvars, catvar_t* predCatvars, binvar_t* trueBinvars, binvar_t* predBinvars, unsigned shuffleCount, unsigned sampleCount){
  continuousErrorModels = new ContinuousErrorModel*[sampleStructure.contvarCount * continuousModelCount];
  for(unsigned featureIndex = 0; featureIndex < sampleStructure.contvarCount; featureIndex++){
    for(unsigned regressorIndex = 0; regressorIndex < continuousModelCount; regressorIndex++){
      
      contvar_t* tc = trueContvars + dataIndex(0             , featureIndex, 0, 0, sampleStructure.contvarCount, shuffleCount + 1, sampleCount);
      contvar_t* pc = predContvars + dataIndex(regressorIndex, featureIndex, 0, 0, sampleStructure.contvarCount, shuffleCount + 1, sampleCount);
      
      *getContinuousModelPtr(featureIndex, regressorIndex) = (*emc.rc)(&sampleStructure, featureIndex, 
        tc, pc, (shuffleCount + 1) * sampleCount);
      
    }
  }
  
  categoricalErrorModels = new CategoricalErrorModel*[sampleStructure.catvarCount * categoricalModelCount];
  for(unsigned featureIndex = 0; featureIndex < sampleStructure.catvarCount; featureIndex++){
    for(unsigned classifierIndex = 0; classifierIndex < categoricalModelCount; classifierIndex++){
      
      catvar_t* tc = trueCatvars + dataIndex(0              , featureIndex, 0, 0, sampleStructure.catvarCount, shuffleCount + 1, sampleCount);
      catvar_t* pc = predCatvars + dataIndex(classifierIndex, featureIndex, 0, 0, sampleStructure.catvarCount, shuffleCount + 1, sampleCount);
      
      *getCategoricalModelPtr(featureIndex, classifierIndex) = (*emc.cc)(&sampleStructure, featureIndex, 
        tc, pc, (shuffleCount + 1) * sampleCount);
      
    }
  }
  

  binaryErrorModels = new BinaryErrorModel*[sampleStructure.binvarCount * binaryModelCount];
  for(unsigned featureIndex = 0; featureIndex < sampleStructure.binvarCount; featureIndex++){
    for(unsigned binaryClassifierIndex = 0; binaryClassifierIndex < binaryModelCount; binaryClassifierIndex++){
      
      binvar_t* tc = trueBinvars + dataIndex(0             , featureIndex, 0, 0, sampleStructure.binvarCount, shuffleCount + 1, sampleCount);
      binvar_t* pc = predBinvars + dataIndex(binaryClassifierIndex, featureIndex, 0, 0, sampleStructure.binvarCount, shuffleCount + 1, sampleCount);
      
      *getBinaryModelPtr(featureIndex, binaryClassifierIndex) = (*emc.bcc)(&sampleStructure, featureIndex, 
        tc, pc, (shuffleCount + 1) * sampleCount);
      
    }
  }
  
}

//Validation set constructor
ErrorModelCollection::ErrorModelCollection(SampleStructure sampleStructure, ErrorModelCreators emc, Array<Sample> trainingData, Array<Sample> validationData, Predictor predictor)
    : ss(sampleStructure), continuousModelCount(predictor.regressorCount), categoricalModelCount(predictor.classifierCount), binaryModelCount(predictor.binaryClassifierCount) {

  assert(!StructuredSampleCollection(sampleStructure, validationData).hasUnknowns());
  
  Sample allData[trainingData.length + validationData.length];
  arrayCopy<Sample>(allData, trainingData.data, trainingData.length);
  arrayCopy<Sample>(allData + trainingData.length, validationData.data, validationData.length);
  
  //Calculate entropy values
  calculateEntropies(Array<Sample>(trainingData.length + validationData.length, allData));
  
  //Memory for predictor results.
  
  unsigned trueContvarsLen = sampleStructure.contvarCount * validationData.length;
  unsigned predContvarsLen = trueContvarsLen * predictor.regressorCount;
  
  contvar_t trueContvars[trueContvarsLen];
  contvar_t predContvars[predContvarsLen];
  
  unsigned trueCatvarsLen = sampleStructure.catvarCount * validationData.length;
  unsigned predCatvarsLen = trueCatvarsLen * predictor.classifierCount;
  
  catvar_t trueCatvars[trueCatvarsLen];
  catvar_t predCatvars[predCatvarsLen];
  
  unsigned trueBinvarLen = sampleStructure.binvarCount * validationData.length;
  unsigned predBinvarLen = trueBinvarLen * predictor.binaryClassifierCount;
  
  binvar_t trueBinvars[trueBinvarLen];
  binvar_t predBinvars[predBinvarLen];
  
  #ifdef DEBUG
  //Set everything to values that can't be produced for testing.
  
  arraySet<contvar_t>(trueContvars, trueContvarsLen, UNKNOWN_CONT);
  arraySet<contvar_t>(predContvars, predContvarsLen, UNKNOWN_CONT);
  
  arraySet<catvar_t>(trueCatvars, trueCatvarsLen, UNKNOWN_CAT);
  arraySet<catvar_t>(predCatvars, predCatvarsLen, UNKNOWN_CAT);
  
  //No available representation for binvars, sorry.
  #endif

  for(unsigned sampleIndex = 0; sampleIndex < validationData.length; sampleIndex++){
    
    for(unsigned featureIndex = 0; featureIndex < ss.contvarCount; featureIndex++){
      //unsigned dataIndex(unsigned predictor, unsigned feature, unsigned shuffle, unsigned sample, unsigned featureCount, unsigned shuffleCount, unsigned sampleCount){

      trueContvars[dataIndex(0, featureIndex, 0, sampleIndex, ss.contvarCount, 1, validationData.length)] =
              validationData.data[sampleIndex].
              contvars[featureIndex];
      for(unsigned predictorIndex = 0; predictorIndex < predictor.regressorCount; predictorIndex++){
        predContvars[dataIndex(predictorIndex, featureIndex, 0, sampleIndex, ss.contvarCount, 1, validationData.length)] =
              predictor.getRegressor(featureIndex, predictorIndex)->predict(validationData[sampleIndex]);
      }
    }

    for(unsigned featureIndex = 0; featureIndex < ss.catvarCount; featureIndex++){

      trueCatvars[dataIndex(0, featureIndex, 0, sampleIndex, ss.catvarCount, 1, validationData.length)] =
              validationData.data[sampleIndex].
              catvars[featureIndex];
      for(unsigned predictorIndex = 0; predictorIndex < predictor.classifierCount; predictorIndex++){
        catvar_t prediction = predictor.getClassifier(featureIndex, predictorIndex)->predict(validationData[sampleIndex]);
        assert(prediction < ss.catvarSizes[featureIndex]);
        assert(isUnknownCatVar(predCatvars[dataIndex(predictorIndex, featureIndex, 0, sampleIndex, ss.catvarCount, 1, validationData.length)]));
        assert(dataIndex(predictorIndex, featureIndex, 0, sampleIndex, ss.catvarCount, 1, validationData.length) < predCatvarsLen);
        predCatvars[dataIndex(predictorIndex, featureIndex, 0, sampleIndex, ss.catvarCount, 1, validationData.length)] = prediction;
      }
    }

    for(unsigned featureIndex = 0; featureIndex < ss.binvarCount; featureIndex++){
      trueBinvars[dataIndex(0, featureIndex, 0, sampleIndex, ss.binvarCount, 1, validationData.length)] =
              validationData.data[sampleIndex].
              binvars[featureIndex];
      for(unsigned predictorIndex = 0; predictorIndex < predictor.binaryClassifierCount; predictorIndex++){
        predBinvars[dataIndex(predictorIndex, featureIndex, 0, sampleIndex, ss.binvarCount, 1, validationData.length)] =
              predictor.getBinaryClassifier(featureIndex, predictorIndex)->predict(validationData[sampleIndex]);
      }
    }
  }
  
  //Make sure that everything actually got set.
  //We do this by checking for unknowns, which will not be produced, so they signify that data is left unset
  #ifdef DEBUG
  for(unsigned i = 0; i < trueContvarsLen; i++){
    if(isUnknownContVar(trueContvars[i])){
      std::cerr << "Unknown True Contvar: " << i << std::endl;
    }
  }
  for(unsigned i = 0; i < predContvarsLen; i++){
    if(isUnknownContVar(predContvars[i])){
      std::cerr << "Unknown Predicted Contvar: " << i << std::endl;
    }
  }
  
  for(unsigned i = 0; i < trueCatvarsLen; i++){
    if(isUnknownCatVar(trueCatvars[i])){
      std::cerr << "Unknown True Catvar: " << i << std::endl;
    }
  }
  for(unsigned i = 0; i < predCatvarsLen; i++){
    if(isUnknownCatVar(predCatvars[i])){
      std::cerr << "Unknown Predicted Catvar: " << i << std::endl;
    }
  }
  #endif
  
  buildErrorModels(sampleStructure, emc, trueContvars, predContvars, trueCatvars, predCatvars, trueBinvars, predBinvars, 0, validationData.length);

}

//Cross Validation Constructor
//Warning: shuffles >= 0 <==> input data is shuffled.
ErrorModelCollection::ErrorModelCollection(MlCreators mlc, ErrorModelCreators emc, StructuredSampleCollection input, unsigned folds, unsigned shuffles)
    : ss(input.sampleStructure), continuousModelCount(mlc.regressorCount), categoricalModelCount(mlc.classifierCount), binaryModelCount(mlc.binaryClassifierCount) {
  
  assert(!input.hasUnknowns());
  
  //Calculate entropy values
  calculateEntropies(input.samples);
  
  //Stack allocate a blob of memory to copy training samples.
  Sample blob[input.samples.length - (input.samples.length / folds)]; //allocate just enough memory to fit all but one fold.

  //Array<Sample> trainFolds = Array<Sample>(blob, 0);
  Array<Sample> valiFolds;
  
  //Memory for predictor results.
  
  unsigned trueContvarsLen = input.sampleStructure.contvarCount * input.samples.length * (shuffles + 1);
  unsigned predContvarsLen = trueContvarsLen * continuousModelCount;
  
  contvar_t trueContvars[trueContvarsLen];
  contvar_t predContvars[predContvarsLen];
  
  unsigned trueCatvarsLen = input.sampleStructure.catvarCount * input.samples.length * (shuffles + 1);
  unsigned predCatvarsLen = trueCatvarsLen * categoricalModelCount;
  
  catvar_t trueCatvars[trueCatvarsLen];
  catvar_t predCatvars[predCatvarsLen];
  
  unsigned trueBinvarLen = input.sampleStructure.binvarCount * input.samples.length * (shuffles + 1);
  unsigned predBinvarLen = trueBinvarLen * binaryModelCount;
  
  binvar_t trueBinvars[trueBinvarLen];
  binvar_t predBinvars[predBinvarLen];
  
  #ifdef DEBUG
  //Set everything to values that can't be produced for testing.
  
  arraySet<contvar_t>(trueContvars, trueContvarsLen, UNKNOWN_CONT);
  arraySet<contvar_t>(predContvars, predContvarsLen, UNKNOWN_CONT);
  
  arraySet<catvar_t>(trueCatvars, trueCatvarsLen, UNKNOWN_CAT);
  arraySet<catvar_t>(predCatvars, predCatvarsLen, UNKNOWN_CAT);
  
  //No available representation for binvars, sorry.
  #endif
  
  std::random_device rd;
  std::default_random_engine random(rd());
    
  for(unsigned shuffleIndex = 0; shuffleIndex <= shuffles; shuffleIndex++){

    //Shuffle the training data for another round of CV.
    input.samples.shuffle(random);
    

    #define THREADED
    #ifdef THREADED

    //Optimization: This is a complete waste, needs to be replaced by a counting iterator of some sort.
#define MAX2(a,b) (((a)>(b))?(a):(b))
#define MAX3(a,b,c) MAX2((a),MAX2((b),(c)))
    unsigned maxCt = MAX3(input.sampleStructure.contvarCount, input.sampleStructure.catvarCount, input.sampleStructure.binvarCount);
    unsigned countIndices [maxCt];
//    std::cerr << maxCt << std::endl << std::endl << std::endl;
    for(unsigned i = 0; i < maxCt; i++){
      countIndices[i] = i;
    }
        
    for(unsigned foldIndex = 0; foldIndex < folds; foldIndex++){
      
      Array<Sample> trainFolds = Array<Sample>(blob, 0);

      //Optimization: Could do with less copying.

      //Copy the data before the current fold
      arrayCopy(trainFolds.data, input.samples.data, foldStart(foldIndex, folds, input.samples.length));
      //Copy the data after the current fold
      arrayCopy(trainFolds.data + foldStart(foldIndex, folds, input.samples.length), input.samples.data + foldStart(foldIndex + 1, folds, input.samples.length), input.samples.length - foldStart(foldIndex + 1, folds, input.samples.length));
      trainFolds.length = input.samples.length - foldStart(foldIndex + 1, folds, input.samples.length) + foldStart(foldIndex, folds, input.samples.length);

      valiFolds = input.samples.slice(foldStart(foldIndex, folds, input.samples.length), foldStart(foldIndex + 1, folds, input.samples.length));

      assert(valiFolds.length + trainFolds.length == input.samples.length); //Train + Vali add up to total.
      assert(foldStart(foldIndex, folds, input.samples.length) + valiFolds.length <= input.samples.length);

      //Build a Predictor
      Predictor predictor = Predictor(input.sampleStructure, trainFolds, mlc);

      for(unsigned sampleIndex = 0; sampleIndex < valiFolds.length; sampleIndex++){
//      for(unsigned featureIndex = 0; featureIndex < ss.contvarCount; featureIndex++){
        std::for_each(countIndices, countIndices + ss.contvarCount, [&](unsigned featureIndex){
          unsigned trueIndex = dataIndexWithFold(0, featureIndex, shuffleIndex, foldIndex, sampleIndex, ss.contvarCount, shuffles + 1, input.samples.length, folds);
          assert(isUnknownContVar(trueContvars[trueIndex]));
          trueContvars[trueIndex] =
                  valiFolds.data[sampleIndex].
                  contvars[featureIndex];
          for(unsigned predictorIndex = 0; predictorIndex < mlc.regressorCount; predictorIndex++){
            contvar_t prediction = predictor.getRegressor(featureIndex, predictorIndex)->predict(valiFolds[sampleIndex]);
            unsigned predIndex = dataIndexWithFold(predictorIndex, featureIndex, shuffleIndex, foldIndex, sampleIndex, ss.contvarCount, shuffles + 1, input.samples.length, folds);
            assert(isUnknownContVar(predContvars[predIndex]));
            assert(predIndex < predContvarsLen);
            predContvars[dataIndexWithFold(predictorIndex, featureIndex, shuffleIndex, foldIndex, sampleIndex, ss.contvarCount, shuffles + 1, input.samples.length, folds)] = prediction;
          }
        }, __gnu_parallel::parallel_balanced);

//      for(unsigned featureIndex = 0; featureIndex < ss.catvarCount; featureIndex++){
        std::for_each(countIndices, countIndices + ss.catvarCount, [&](unsigned featureIndex){
          unsigned trueIndex = dataIndexWithFold(0, featureIndex, shuffleIndex, foldIndex, sampleIndex, ss.catvarCount, shuffles + 1, input.samples.length, folds);
          assert(isUnknownCatVar(trueCatvars[trueIndex]));
          trueCatvars[trueIndex] =
                  valiFolds.data[sampleIndex].
                  catvars[featureIndex];
          for(unsigned predictorIndex = 0; predictorIndex < mlc.classifierCount; predictorIndex++){
            catvar_t prediction = predictor.getClassifier(featureIndex, predictorIndex)->predict(valiFolds[sampleIndex]);
            assert(prediction < ss.catvarSizes[featureIndex]);

            unsigned predIndex = dataIndexWithFold(predictorIndex, featureIndex, shuffleIndex, foldIndex, sampleIndex, ss.catvarCount, shuffles + 1, input.samples.length, folds);
            assert(isUnknownCatVar(predCatvars[predIndex]));
            assert(predIndex < predCatvarsLen);
            predCatvars[predIndex] = prediction;
          }
        }, __gnu_parallel::parallel_balanced);

//      for(unsigned featureIndex = 0; featureIndex < ss.binvarCount; featureIndex++){
        std::for_each(countIndices, countIndices + ss.binvarCount, [&](unsigned featureIndex){
          trueBinvars[dataIndexWithFold(0, featureIndex, shuffleIndex, foldIndex, sampleIndex, ss.binvarCount, shuffles + 1, input.samples.length, folds)] =
                  valiFolds.data[sampleIndex].
                  binvars[featureIndex];
          for(unsigned predictorIndex = 0; predictorIndex < mlc.binaryClassifierCount; predictorIndex++){
            predBinvars[dataIndexWithFold(predictorIndex, featureIndex, shuffleIndex, foldIndex, sampleIndex, ss.binvarCount, shuffles + 1, input.samples.length, folds)] = predictor.getBinaryClassifier(featureIndex, predictorIndex)->predict(valiFolds[sampleIndex]);
          }
        }, __gnu_parallel::parallel_balanced);
      }

      //Free the predictor memory
      predictor.freeMemory();

    }
    
    #else

    for(unsigned foldIndex = 0; foldIndex < folds; foldIndex++){

      Array<Sample> trainFolds = Array<Sample>(blob, 0);

      //Optimization: Could do with less copying.

      //Copy the data before the current fold
      arrayCopy(trainFolds.data, input.samples.data, foldStart(foldIndex, folds, input.samples.length));
      //Copy the data after the current fold
      arrayCopy(trainFolds.data + foldStart(foldIndex, folds, input.samples.length), input.samples.data + foldStart(foldIndex + 1, folds, input.samples.length), input.samples.length - foldStart(foldIndex + 1, folds, input.samples.length));
      trainFolds.length = input.samples.length - foldStart(foldIndex + 1, folds, input.samples.length) + foldStart(foldIndex, folds, input.samples.length);

      valiFolds = input.samples.slice(foldStart(foldIndex, folds, input.samples.length), foldStart(foldIndex + 1, folds, input.samples.length));

      assert(valiFolds.length + trainFolds.length == input.samples.length);
      assert(foldStart(foldIndex, folds, input.samples.length) + valiFolds.length <= input.samples.length);

      //Build a Predictor
      Predictor predictor = Predictor(input.sampleStructure, trainFolds, mlc);

      for(unsigned sampleIndex = 0; sampleIndex < valiFolds.length; sampleIndex++){
        for(unsigned featureIndex = 0; featureIndex < ss.contvarCount; featureIndex++){
          unsigned trueIndex = dataIndexWithFold(0, featureIndex, shuffleIndex, foldIndex, sampleIndex, ss.contvarCount, shuffles + 1, input.samples.length, folds);
          assert(isUnknownContVar(trueContvars[trueIndex]));
          trueContvars[trueIndex] =
                  valiFolds.data[sampleIndex].
                  contvars[featureIndex];
          for(unsigned predictorIndex = 0; predictorIndex < mlc.regressorCount; predictorIndex++){
            contvar_t prediction = predictor.getRegressor(featureIndex, predictorIndex)->predict(valiFolds[sampleIndex]);
            unsigned predIndex = dataIndexWithFold(predictorIndex, featureIndex, shuffleIndex, foldIndex, sampleIndex, ss.contvarCount, shuffles + 1, input.samples.length, folds);
            assert(isUnknownContVar(predContvars[predIndex]));
            assert(predIndex < predContvarsLen);
            predContvars[dataIndexWithFold(predictorIndex, featureIndex, shuffleIndex, foldIndex, sampleIndex, ss.contvarCount, shuffles + 1, input.samples.length, folds)] = prediction;
          }
        }

        for(unsigned featureIndex = 0; featureIndex < ss.catvarCount; featureIndex++){
          unsigned trueIndex = dataIndexWithFold(0, featureIndex, shuffleIndex, foldIndex, sampleIndex, ss.catvarCount, shuffles + 1, input.samples.length, folds);
          assert(isUnknownCatVar(trueCatvars[trueIndex]));
          trueCatvars[trueIndex] =
                  valiFolds.data[sampleIndex].
                  catvars[featureIndex];
          for(unsigned predictorIndex = 0; predictorIndex < mlc.classifierCount; predictorIndex++){
            catvar_t prediction = predictor.getClassifier(featureIndex, predictorIndex)->predict(valiFolds[sampleIndex]);
            assert(prediction < ss.catvarSizes[featureIndex]);

            unsigned predIndex = dataIndexWithFold(predictorIndex, featureIndex, shuffleIndex, foldIndex, sampleIndex, ss.catvarCount, shuffles + 1, input.samples.length, folds);
            assert(isUnknownCatVar(predCatvars[predIndex]));
            assert(predIndex < predCatvarsLen);
            predCatvars[predIndex] = prediction;
          }
        }

        for(unsigned featureIndex = 0; featureIndex < ss.binvarCount; featureIndex++){
          trueBinvars[dataIndexWithFold(0, featureIndex, shuffleIndex, foldIndex, sampleIndex, ss.binvarCount, shuffles + 1, input.samples.length, folds)] =
                  valiFolds.data[sampleIndex].
                  binvars[featureIndex];
          for(unsigned predictorIndex = 0; predictorIndex < mlc.binaryClassifierCount; predictorIndex++){
            predBinvars[dataIndexWithFold(predictorIndex, featureIndex, shuffleIndex, foldIndex, sampleIndex, ss.binvarCount, shuffles + 1, input.samples.length, folds)] = predictor.getBinaryClassifier(featureIndex, predictorIndex)->predict(valiFolds[sampleIndex]);
          }
        }
      }

      //Free the predictor memory
      predictor.freeMemory();
    }
    #endif
  }
  
  //Make sure that everything actually got set.
  //We do this by checking for unknowns, which will not be produced, so they signify that data is left unset
  #ifdef DEBUG
  for(unsigned i = 0; i < trueContvarsLen; i++){
    if(isUnknownContVar(trueContvars[i])){
      std::cerr << "Unknown True Contvar: " << i << std::endl;
    }
  }
  for(unsigned i = 0; i < predContvarsLen; i++){
    if(isUnknownContVar(predContvars[i])){
      std::cerr << "Unknown Predicted Contvar: " << i << std::endl;
    }
  }
  
  for(unsigned i = 0; i < trueCatvarsLen; i++){
    if(isUnknownCatVar(trueCatvars[i])){
      std::cerr << "Unknown True Catvar: " << i << std::endl;
    }
  }
  for(unsigned i = 0; i < predCatvarsLen; i++){
    if(isUnknownCatVar(predCatvars[i])){
      std::cerr << "Unknown Predicted Catvar: " << i << std::endl;
    }
  }
  #endif

  buildErrorModels(input.sampleStructure, emc, trueContvars, predContvars, trueCatvars, predCatvars, trueBinvars, predBinvars, shuffles, input.samples.length);  
}

void ErrorModelCollection::calculateEntropies(Array<Sample> samples) {
  entropyValues = new fracfloat_t[ss.featureCount()];
  
  fracfloat_t contvars[samples.length];

  for(unsigned i = 0; i < ss.contvarCount; i++){
    samples.mapTo<fracfloat_t, unsigned>([](Sample s, unsigned fIndex){return s.contvars[fIndex];}, i, Array<fracfloat_t>(contvars, samples.length));
    
    //Build  distribution
    
    //Optimization: Lots of news and deletes are going on here.
    //Detail: the choice of error model.
    MultiGaussianWeighted dist = MultiGaussianWeighted::fitGaussianKernel(contvars, samples.length, (unsigned)sqrt(samples.length));
    *continuousFeatureEntropy(i) = dist.approximateDifferentialEntropyFromSamples(Array<fracfloat_t>(contvars, samples.length));
    dist.freeMemory();
  }
  
  //catvar_t catvars[samples.length];
  
  for(unsigned i = 0; i < ss.catvarCount; i++){
    //Count the rate of occurrences of each catvar
    fracfloat_t dist[ss.catvarSizes[i]];
    arrayZero<fracfloat_t>(dist, ss.catvarSizes[i]);
    for(unsigned j = 0; j < samples.length; j++){
      dist[samples[j].catvars[i]] += 1.0 / samples.length;
    }
    
    //Now take the entropy
    
    //Detail: Would applying Laplacian smoothing here be logical?
    //Detail: Numerics can cause this to be negative on a completely skewed distribution.  Fix: Add a small constant FRACFLOAT_EPSILON
    *categoricalFeatureEntropy(i) = entropy<fracfloat_t>(dist, ss.catvarSizes[i]) + FRACFLOAT_EPSILON * 10;
  }
  for(unsigned i = 0; i < ss.binvarCount; i++){
    fracfloat_t dist[2];
    arrayZero<fracfloat_t>(dist, 2);
    for(unsigned j = 0; j < samples.length; j++){
      dist[samples[j].binvars[i]] += 1.0 / samples.length;
    }
    
    //Now take the entropy
    
    //Detail: Would applying Laplacian smoothing here be logical?
    *binaryFeatureEntropy(i) = entropy<fracfloat_t>(dist, 2);
  }
  
  //And the entropies of each feature have been calculated.
  
  //Make sure all differential entropy values are finite.
  assert(Array<fracfloat_t>(entropyValues, ss.contvarCount).conjunction([](fracfloat_t f){return f > FRACFLOAT_NEGATIVE_INFINITY && f < FRACFLOAT_INFINITY;}));

  //Make sure all discrete entropy values are nonnegative.
  assert(Array<fracfloat_t>(entropyValues + ss.contvarCount, ss.catvarCount + ss.binvarCount).conjunction([](fracfloat_t f){return f >= 0 && f < FRACFLOAT_INFINITY;}));
}

void ErrorModelCollection::freeMemory() {
  
  Array<ContinuousErrorModel*>(continuousErrorModels, ss.contvarCount * continuousModelCount).forEach([](ContinuousErrorModel*& m){m->freeMemory(); delete m;});
  delete [] continuousErrorModels;

  Array<CategoricalErrorModel*>(categoricalErrorModels, ss.catvarCount * categoricalModelCount).forEach([](CategoricalErrorModel*& m){m->freeMemory(); delete m;});
  delete [] categoricalErrorModels;

  Array<BinaryErrorModel*>(binaryErrorModels, ss.binvarCount * binaryModelCount).forEach([](BinaryErrorModel*& m){m->freeMemory(); delete m;});
  delete [] binaryErrorModels;
  
  delete[] entropyValues;

}