#include "io.hpp"
#include "vectormath.hpp"

#include <string>
#include <sstream>
#include <locale>
#include <vector>
#include <unordered_map>

#include <algorithm>
#include <functional> 
#include <cctype>
#include <locale>
#include <iterator>

/*
#define HAVE_SDL_SDL_H
#include <regex>
*/

///////////////////
//File IO Functions


//File IO helpers
//These are provided to give default error messages on loads

void openOutputFile(std::ofstream& s, std::string path){
  s.open(path);
  if(s.fail()){
    std::cerr << "Failure opening file \"" << path << "\" for output." << std::endl;
    exit(0);
  }
}

void openInputFile(std::ifstream& s, std::string path){
  s.open(path);
  if(s.fail()){
    std::cerr << "Failure opening file \"" << path << "\" for input." << std::endl;
    exit(0);
  }
}

//////////////////
//String Functions

std::vector<std::string> split(const std::string& s, const std::string& delim, const bool keep_empty) {
  std::vector<std::string> result;
  if (delim.empty()) {
    result.push_back(s);
    return result;
  }
  std::string::const_iterator substart = s.begin(), subend;
  while (true) {
    //Optimization: Quality: Consider making this compatible with serial

    #ifdef _GLIBCXX_PARALLEL
    subend = std::search(substart, s.end(), delim.begin(), delim.end(), __gnu_parallel::sequential_tag());
    #else
    subend = std::search(substart, s.end(), delim.begin(), delim.end());
    #endif
    std::string temp(substart, subend);
    if (keep_empty || !temp.empty()) {
      result.push_back(temp);
    }
    if (subend == s.end()) {
      break;
    }
    substart = subend + delim.size();
  }
  return result;
}

//trim from: http://stackoverflow.com/questions/216823/whats-the-best-way-to-trim-stdstring

// trim from start
static std::string &ltrim(std::string &s) {
  s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
  return s;
}

// trim from end
static std::string &rtrim(std::string &s) {
  s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
  return s;
}

// trim from both ends
std::string &trim(std::string &s) {
  return ltrim(rtrim(s));
}

void tolower(std::string& s){
  for(unsigned i = 0; i < s.length(); i++){
    s[i] = tolower(s[i]);
  }
}

//Used to sanitize some outputs
std::string spaceToUnderscore(std::string s){
  std::string newStr;
  newStr.resize(s.length());
  for(unsigned i = 0; i < s.length(); i++){
    if(s[i] == ' ') newStr[i] = '_';
    else newStr[i] = s[i];
  }
  return newStr;
}

//Error output function
std::ostream& parserError(unsigned lineNum){
  std::cerr << "ERROR: Line " << lineNum << ": ";
  return std::cerr;
}

//Reading functions

//Generic data read:
StructuredSampleCollection readData(std::string splitStr, std::istream& in, unsigned lineNum, SampleStructure st, FeatureOrdering& featureTypes){
  
  std::unordered_map<std::string, catvar_t> catLookups[st.catvarCount];
  for(unsigned i = 0; i < st.catvarCount; i++){
    catLookups[i].reserve(st.catvarSizes[i]);
    for(unsigned j = 0; j < st.catvarSizes[i]; j++){
      catLookups[i][st.catvarCatNames[i][j]] = (catvar_t)j;
      //std::cout << "CATVAR: (" << i << ", " << j << ") \"" << catvarCatNames[i][j] << "\"." << std::endl;
    }
  }
  
  //Code for binary classes (these are resolved later).
  /*
  std::unordered_map<std::string, binvar_t> binLookups[st.binvarCount]; //Optimization actually don't use an unordered map for a 2 element lookup...
  for(unsigned i = 0; i < binvarNames.size(); i++){
    binLookups[i].reserve(2);
    for(unsigned j = 0; j < 2; j++){
      binLookups[i][binvarCatNames[i][j]] = (binvar_t)j;
    }
  }
   */
  
  unsigned lineSize = st.featureCount();
  for(unsigned i = 0; i < featureTypes.size(); i++){
    if(featureTypes[i] == feature_ignore) lineSize++;
  }

  //Vector to store samples
  std::vector<Sample> samples;
  
  std::string line;
  while (lineNum++, std::getline(in, line)){
    //Filter comments, blank lines
    if(line.length() == 0 || line[0] == '%') continue;
    std::vector<std::string> lineSplit = split(line, splitStr);
    
    #ifndef UNSAFE
    if(lineSplit.size() != lineSize){
      parserError(lineNum) << "Found " << lineSplit.size() << " features, need " << lineSize << "." << std::endl;
      exit(1);
    }
    #endif
    
    fracfloat_t* contvars = new fracfloat_t[st.contvarCount];
    catvar_t* catvars = new catvar_t[st.catvarCount];
    std::vector<bool> binvars = std::vector<bool>();
    binvars.reserve(st.binvarCount);
    
    unsigned contvarIndex = 0;
    unsigned catvarIndex = 0;
    for(unsigned i = 0; i < lineSize; i++){
      
      switch(featureTypes[i]){
        case feature_contvar:
        {
          #ifdef UNKNOWNVALUES
          if(lineSplit[i] == "?"){
            contvars[contvarIndex] = UNKNOWN_CONT;
          }
          #else
          if(false){}
          #endif
          else{
            #ifdef UNSAFE
            //Unsafe (but fast) parse.
            contvars[contvarIndex] = (fracfloat_t) ::atof(lineSplit[i].c_str());
            #else
            contvars[contvarIndex] = (fracfloat_t) std::stod(lineSplit[i], NULL);
            #endif
          }
          contvarIndex++;
        } 
        break;
        case feature_catvar:
        { //Only other type is CATVAR
          std::string key = lineSplit[i];
          //Optimization: slow double lookup.  Shame on C++0x team, it took them until 2011 and they still did a terrible job!
          if(catLookups[catvarIndex].count(key)){
            catvars[catvarIndex] = catLookups[catvarIndex][key];
          #ifdef UNKNOWNVALUES
          } else if(key == "?"){
            catvars[catvarIndex] = UNKNOWN_CAT;
          #endif
          } else {
            parserError(lineNum) << "Don't recognize category \"" << key << "\" for categorical feature \"" << st.catvarName(i) << "\"" << std::endl;
            exit(1);
          }
          catvarIndex++;
        }
        break;
        default:
        {
          //Ignore feature.
          assert(featureTypes[i] == feature_ignore);
        }
      }
      //Old binvar code (now binvars just get split from catvars later).
      /*
       std::string key = lineSplit[i + st.contvarCount + st.catvarCount];
      //Optimization: slow double lookup.
      if(binLookups[i].count(key)) binvars[i] = binLookups[i][key];
      else{
        parserError(lineNum) << "Don't recognize category \"" << key << "\" for binary feature \"" << catvarNames[i] << "\"" << std::endl;
        exit(1);
      }
       */
    }
    
    //Note, a BitArray is just an std::vector<bool>, so we can just use it.
    
    //There will be no binvar unknowns because there are no binvars.
    samples.push_back(Sample(contvars, catvars, binvars, std::vector<unsigned>()));
  }
  
  //Array<Sample> samplesArr(samples);
  return StructuredSampleCollection(st, samples); //Uses the array constructor that copies data from a vector.  
}

//ARFF

//Read function

//Note: could possibly use Waffles arff parser
//http://waffles.sourceforge.net/docs/matrices.html
StructuredSampleCollection readArff(std::istream& in){
  unsigned lineNum = 0;
  
  std::string relationName;
  std::vector<std::string> contvarNames;
  std::vector<std::string> catvarNames;
  //std::vector<std::string> binvarNames;
  
  std::vector<Array<std::string> > catvarCatNames;
  //std::vector<Array<std::string> > binvarCatNames;
  
  std::vector<featurety_t> featureTypes; //Used to determine the order of the arff arguments.
  
  std::string line;
  while (1)
  {
    lineNum++;
    if(!std::getline(in, line)){
      parserError(lineNum) << "Blank or absent @DATA section." << std::endl;
      exit(1);
    }
    
    //Filter comments, blank lines
    if(line.length() == 0 || line[0] == '%') continue;
    std::istringstream lineStream(line);

    //Read from the line
    std::string token;
    lineStream >> token;

    tolower(token);
    if(token == "@relation"){
      lineStream >> relationName;
    }
    else if(token == "@attribute"){
      std::string attributeName;
      lineStream >> attributeName;
      
      lineStream >> token;
      
      //std::cout << "ATTR TY: " << "\"" << token << "\"" <<std::endl;
      if(token[0] == '{'){
        
        //Trim the {
        token = token.substr(1);
        
        std::string rest;
        //Read the rest of the line into rest, then concat with token
        getline(lineStream, rest);

        token = token + rest;
        
        //Split on ','
        
        std::vector<std::string> categories = split(split(token, "}")[0], ","); //split off everything after the '}', then split the remainder along commas.
        for(unsigned i = 0; i < categories.size(); i++){
          categories[i] = trim(categories[i]); //Trim the space from each one.
        }
        
        //Note: binvars get resolved at a later phase.
        
        catvarNames.push_back(attributeName);
        
        //Optimization: This copying could be eliminated.
        Array<std::string> catsArr = Array<std::string>(categories.size());
        arrayCopy<std::string>(catsArr.data, categories.data(), categories.size());
        catvarCatNames.push_back(catsArr);
        featureTypes.push_back(feature_catvar);
      }
      else{
        tolower(token);
        if(token == "numeric" || token == "real"){
          contvarNames.push_back(attributeName);
          featureTypes.push_back(feature_contvar);
        }
        else{
          parserError(lineNum) << "Don't recognize feature type \"" << token << "\"." << std::endl;
          exit(1);
        }
      }
    }
    else if(token == "@data"){
      break;
    }
  }
  
  std::string** catvarNamesArr = new std::string*[catvarNames.size()];
  for(unsigned i = 0; i < catvarNames.size(); i++){
    //Safety: Validate no identical category names.
    catvarNamesArr[i] = catvarCatNames[i].data;
  }
  
  /*
  std::string* binvarNamesArr = new std::string[binvarNames.size() * 2];
  for(unsigned i = 0; i < binvarNames.size(); i++){
    //Safety: Validate no identical category names.
    binvarNamesArr[i * 2 + 0] = binvarCatNames[i].data[0];
    binvarNamesArr[i * 2 + 1] = binvarCatNames[i].data[1];
  }
   */
  
  unsigned* catvarSizes = new unsigned[catvarNames.size()];
  for(unsigned i = 0; i < catvarNames.size(); i++){
    catvarSizes[i] = catvarCatNames[i].length;
  }
  
  //Safety: Validate no identical names.
  std::string* allNames = new std::string[contvarNames.size() + catvarNames.size() /* + binvarNames.size() */];
  arrayCopy<std::string>(allNames, contvarNames.data(), contvarNames.size());
  arrayCopy<std::string>(allNames + contvarNames.size(), catvarNames.data(), catvarNames.size());
  //arrayCopy<std::string>(allNames + contvarNames.size() + catvarNames.size(), binvarNames.data(), binvarNames.size());
  
  SampleStructure st = SampleStructure(contvarNames.size(), catvarNames.size(), 0 /*binvarNames.size()*/, catvarSizes, catvarNamesArr, NULL /* binvarNamesArr */, allNames, relationName);
  
  //Load the data section
  
  return readData(",", in, lineNum, st, featureTypes);
}

//Convenience function: wrap fileIO

StructuredSampleCollection readArff(std::string fileName){
  std::ifstream ifs;

  openInputFile(ifs, fileName);

  StructuredSampleCollection ssc = readArff(ifs);

  ifs.close();

  return ssc;
}

//////////////////
//ARFF writing code.


//Helper Functions

static void writeSampleStructure(std::ostream& out, SampleStructure ss){
  out << "%Generated ARFF file" << std::endl;
  //out << "%" << ss.contvarCount << " continuous features, " << ss.catvarCount << " categorical features, and " << ss.binvarCount << " binary features." << std::endl;
  //out << "%" << ssc.countUnknowns << " unknown values." << std::endl;
  out << "@RELATION " << ss.name << std::endl << std::endl;
  out << "%Features:" << std::endl << std::endl;
  for(unsigned i = 0; i < ss.contvarCount; i++){
    out << "@ATTRIBUTE " << spaceToUnderscore(ss.contvarName(i)) << " NUMERIC" << std::endl;
  }
  for(unsigned i = 0; i < ss.catvarCount; i++){
    out << "@ATTRIBUTE " << spaceToUnderscore(ss.catvarName(i)) << " {";
    out << spaceToUnderscore(*ss.catvarCatNameLookup(i, 0));
    for(unsigned j = 1; j < ss.catvarSizes[i]; j++){
      out << ", " << spaceToUnderscore(*ss.catvarCatNameLookup(i, j));
    }
    out << "}" << std::endl;
  }
  for(unsigned i = 0; i < ss.binvarCount; i++){
    out << "@ATTRIBUTE " << spaceToUnderscore(ss.binvarName(i)) << " {" << spaceToUnderscore(*ss.binvarCatNameLookup(i, 0)) << ", " << spaceToUnderscore(*ss.binvarCatNameLookup(i, 1)) << "}" << std::endl;
  }
}

static void writeSamples(std::ostream& out, StructuredSampleCollection ssc){  
  SampleStructure ss = ssc.sampleStructure;
  
  out << "@DATA" << std::endl;
  for(unsigned i = 0; i < ssc.samples.length; i++){
    bool doneFirst = false;
    if(ss.contvarCount > 0){
      out << ssc.samples[i].contvars[0];
      doneFirst = true;
      for(unsigned j = 1; j < ss.contvarCount; j++){
        out << "," << ssc.samples[i].contvars[j];
      }
    }
    //Detail: quote wrap categories for greater ARFF compliance
    if(ss.catvarCount > 0){
      if(doneFirst) out << ",";
      out << *ss.catvarCatNameLookup(0, ssc.samples[i].catvars[0]);
      doneFirst = true;
      for(unsigned j = 1; j < ss.catvarCount; j++){
        out << "," << *ss.catvarCatNameLookup(j, ssc.samples[i].catvars[j]);
      }
    }
    if(ss.binvarCount > 0){
      if(doneFirst) out << ",";
      out << *ss.binvarCatNameLookup(0, ssc.samples[i].binvars[0]);
      doneFirst = true;
      for(unsigned j = 1; j < ss.binvarCount; j++){
        out << "," << *ss.binvarCatNameLookup(j, ssc.samples[i].binvars[j]);
      }
    }
    out << std::endl;
  }
}

static void writeScoredSamples(std::ostream& out, SampleStructure ss, Array<SampleScorePair> scoredSamples){
  out << "@DATA" << std::endl;
  for(unsigned i = 0; i < scoredSamples.length; i++){
    for(unsigned j = 0; j < ss.contvarCount; j++){
      #ifdef UNKNOWNVALUES
      if(scoredSamples[i].sample.contvarUnknown(j)) out << "?";
      else out << scoredSamples[i].sample.contvars[j];
      #else
      out << scoredSamples[i].sample.contvars[j];
      #endif
      out << ",";
    }
    
    for(unsigned j = 0; j < ss.catvarCount; j++){
      //Avert your eyes.
      out << (
      #ifdef UNKNOWNVALUES
      scoredSamples[i].sample.catvarUnknown(j) ? "?" :
      #endif
      *ss.catvarCatNameLookup(j, scoredSamples[i].sample.catvars[j])) << ",";
    }
    for(unsigned j = 0; j < ss.binvarCount; j++){
      out << (
      #ifdef UNKNOWNVALUES
      scoredSamples[i].sample.binvarUnknown(j) ? "?" :
      #endif
      *ss.binvarCatNameLookup(j, scoredSamples[i].sample.binvars[j])) << ",";
    }
    out << scoredSamples[i].anomalyScore;
    out << std::endl;
  }
}

/////////////////
//Write Functions

void writeArff(std::ostream& out, StructuredSampleCollection ssc){
  writeSampleStructure(out, ssc.sampleStructure);
  out << std::endl;
  writeSamples(out, ssc);  
}

void writeArff(std::ostream& out, SampleStructure structure, Array<SampleScorePair> scoredSamples){
  writeSampleStructure(out, structure);

  out << std::endl;
  out << "@ATTRIBUTE" << " " << "FRaC_Anomaly_Score" << " " << "NUMERIC" << std::endl;
  out << std::endl;

  writeScoredSamples(out, structure, scoredSamples);
}

