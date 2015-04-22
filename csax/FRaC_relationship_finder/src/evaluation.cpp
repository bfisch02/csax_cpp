
#include <iostream>

#include "io.hpp"
#include "evaluation.hpp"
#include "vectormath.hpp"
#include "sample.hpp"

//File loading:
//A bit of a monolith, but it's test code and it gets the job done.

TestFramework loadTestData(std::string testDirectory) {

  //LOAD SAMPLE STRUCTURE
  std::ifstream in;

  in.open(testDirectory + "/metadata", std::ifstream::in);

  unsigned lineNum = 0;
  std::string line;

  int classIndex = -1;

  std::vector<std::string> contvarNames;
  std::vector<std::string> catvarNames;

  std::vector<Array<std::string> > catvarCatNames;


  FeatureOrdering featureTypes;
  while (lineNum++, std::getline(in, line)) {
    line = trim(line);
    if (line.length() == 0 || line[0] == '#') continue;

    std::vector<std::string> lineSplit = split(line, "\t");

    //Check number
    if (lineSplit.size() != 3 && lineSplit[1] != "ignore") {
      parserError(lineNum) << "Metadata file requires index, type, and values." << std::endl;
      std::cerr << "\"" << line << "\" split into " << lineSplit.size() << "." << std::endl;
      exit(1);
    }

    //This requirement precludes named features.
    #if 0
    //Check ordered indices
    if (std::atoi(lineSplit[0].c_str()) != (int) featureTypes.size() + 1) {
      parserError(lineNum) << "Metadata file requires ordered indices." << std::endl;
      exit(1);
    }
    #endif

    if (lineSplit[1] == "real") {
      featureTypes.push_back(feature_contvar);
      contvarNames.push_back(lineSplit[0]);
      //Ignore min/max data (lineSplit[2]);
    } else if (lineSplit[1] == "nominal") {
      std::vector<std::string> catNames = split(lineSplit[2], ",");

      catvarCatNames.push_back(Array<std::string>(catNames));
      featureTypes.push_back(feature_catvar);

      catvarNames.push_back(lineSplit[0]);
    } else if (lineSplit[1] == "class") {
      if (classIndex != -1) {
        parserError(lineNum) << "Metadata file contains multiple class features." << std::endl;
        exit(1);
      }

      classIndex = catvarNames.size();

      std::vector<std::string> catNames = split(lineSplit[2], ",");

      catvarCatNames.push_back(Array<std::string>(catNames));
      featureTypes.push_back(feature_catvar);

      catvarNames.push_back(lineSplit[0]);
    } else if (lineSplit[1] == "ignore") {
      featureTypes.push_back(feature_ignore);
    }
  }
  
  //Make sure there's a class.
  if(classIndex == -1){
    std::cerr << "Metadata file contains no class entry." << std::endl;
    exit(1);
  }

  std::string** catvarNamesArr = new std::string*[catvarNames.size()];
  for (unsigned i = 0; i < catvarNames.size(); i++) {
    //Safety: Validate no identical category names.
    catvarNamesArr[i] = catvarCatNames[i].data;
  }

  unsigned* catvarSizes = new unsigned[catvarNames.size()];
  for (unsigned i = 0; i < catvarNames.size(); i++) {
    catvarSizes[i] = catvarCatNames[i].length;
  }

  std::string* allNames = new std::string[contvarNames.size() + catvarNames.size() /* + binvarNames.size() */];
  #ifdef DEBUG
  arraySet<std::string>(allNames, contvarNames.size() + catvarNames.size(), "\tDebug");
  #endif
  arrayCopy<std::string>(allNames, contvarNames.data(), contvarNames.size());
  arrayCopy<std::string>(allNames + contvarNames.size(), catvarNames.data(), catvarNames.size());
  //arrayCopy<std::string>(allNames + contvarNames.size() + catvarNames.size(), binvarNames.data(), binvarNames.size());

  #ifdef DEBUG
  assert(Array<std::string>(allNames, contvarNames.size() + catvarNames.size()).conjunction([](std::string n){return (n != "\tDebug") && (n.length() > 0);}));
  #endif
  
  SampleStructure st = SampleStructure(contvarNames.size(), catvarNames.size(), 0 /*binvarNames.size()*/, catvarSizes, catvarNamesArr, NULL /* binvarNamesArr */, allNames, "Test");
  
  
  in.close();

  in.open(testDirectory + "/trainset");

  StructuredSampleCollection training = readData("\t", in, 0, st, featureTypes);

  in.close();

  in.open(testDirectory + "/testset");

  StructuredSampleCollection test = readData("\t", in, 0, st, featureTypes);

  in.close();

  //Confirm that all test have the same class.

  catvar_t normalClass = training.samples.head().catvars[classIndex];

  if(isUnknownCatVar(normalClass)){
    std::cerr << "Error: Found unknown class in training data.  All class values must be known." << std::endl;
    exit(1);
  }
  unsigned cl[2]; //weird closure, need both the class index and the normal class value.

  cl[0] = normalClass;
  cl[1] = classIndex;

  if(!(training.samples.conjunction<unsigned*>([](Sample s, unsigned* cl) { return s.catvars[cl[1]] == cl[0]; }, cl))){
    std::cerr << "Error: Training data contains nonidentical class values.  All classes must be normal in training data." << std::endl;
    exit(1);
  }
  
  //Make sure there are no unknown classes in the testset.
  if(test.samples.disjunction<unsigned>([](Sample s, unsigned cl) { return s.catvarUnknown(cl); }, classIndex)){
    std::cerr << "Error: Found unknown class in test data.  All classes must be known in test data." << std::endl;
    exit(1);
  }
  
  //Map normals to true and abnormals to false.
  Array<bool> testClasses = test.samples.map<bool, unsigned*>([](Sample s, unsigned* data) {
    return s.catvars[data[1]] == data[0];
  }, cl);

  //Now modify the sample structure

  for (unsigned i = classIndex + 1; i < st.catvarCount; i++) {
    st.catvarCatNames[i - 1] = st.catvarCatNames[i];
    st.catvarSizes[i - 1] = st.catvarSizes[i];
    st.nameLookup[st.contvarCount + i - 1] = st.nameLookup[st.contvarCount + i]; //NOTE: This code relies heavily on knowledge of the internal representation of the catvar.
  }

  //Now update samples and training data


  cl[0] = classIndex;
  cl[1] = st.catvarCount - classIndex - 1;

  training.samples.forEach<unsigned*>([](Sample& s, unsigned* cl) {
    arrayCopy<catvar_t>(s.catvars + cl[0], s.catvars + cl[0] + 1, cl[1]);
  }, cl); //Shift over the catvars
  test.samples.forEach<unsigned*>([](Sample& s, unsigned* cl) {
    arrayCopy<catvar_t>(s.catvars + cl[0], s.catvars + cl[0] + 1, cl[1]);
  }, cl); //Shift over the catvars

  st.catvarCount--;

  //Delete the memory holding the catvar names for the class
  catvarCatNames[classIndex].freeMemory();
  
  return TestFramework(st, test.samples, training.samples, testClasses);
}
