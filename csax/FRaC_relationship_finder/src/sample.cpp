#include "sample.hpp"
#include "vectormath.hpp"

#include <iostream>


////////
//OUTPUT

std::ostream& operator<<(std::ostream& o, const SampleStructure& s)
{
  o << "{ {";
  if(s.contvarCount > 0){
    o << "\"" << s.contvarName(0) << "\"";
    for(unsigned i = 1; i < s.contvarCount; i++){
      o << ", " << "\"" << s.contvarName(i) << "\"";
    }
  }
  o << "}, {";
  if(s.catvarCount > 0){
    o << "(" << "\"" << s.catvarName(0) << "\"" << ", " << s.catvarSizes[0] << ")";
    for(unsigned i = 1; i < s.catvarCount; i++){
      o << ", " << "(" << "\"" << s.catvarName(i) << "\"" << ", " << s.catvarSizes[i] << ")";
    }
  }
  o << "}, {";
  if(s.binvarCount > 0){
    o << "\"" << s.binvarName(0) << "\"";
    for(unsigned i = 1; i < s.binvarCount; i++){
      o << ", " << "\"" << s.binvarName(i) << "\"";
    }
  }
  o << "} };";
  return o;
}

void writeSample(std::ostream& o, const Sample& s, const SampleStructure& st){
  o << "{ {";
  if(st.contvarCount > 0){
    o << "\"" << s.contvars[0] << "\"";
    for(unsigned i = 1; i < st.contvarCount; i++){
      o << ", " << "\"" << s.contvars[i] << "\"";
    }
  }
  o << "}, {";
  if(st.catvarCount > 0){
    o << "\"" << ((double)s.catvars[0]) << "\"";
    for(unsigned i = 1; i < st.catvarCount; i++){
      o << ", " << "\"" << ((double)s.catvars[i]) << "\"";
    }
  }
  o << "}, {";
  if(st.binvarCount > 0){
    o << "\"" << ((double)s.binvars[0]) << "\"";
    for(unsigned i = 1; i < st.binvarCount; i++){
      o << ", " << "\"" << ((double)s.binvars[i]) << "\"";
    }
  }
  o << "} };";
}

void writeSampleArr(std::ostream& out, const Array<Sample>& samples, const SampleStructure& st){
  out << "{";// << " " << ((double)samples.length);
  for(unsigned i = 0; i < samples.length; i++){
    out << "\n";
    writeSample(out, samples[i], st);
  }
  out << "\n}";
}

bool sampleEq(const SampleStructure ss, const Sample s0, const Sample s1) {
  for(unsigned i = 0; i < ss.contvarCount; i++){
    if(s0.contvars[i] != s1.contvars[i]) return false;
  }
  for(unsigned i = 0; i < ss.catvarCount; i++){
    if(s0.catvars[i] != s1.catvars[i]) return false;
  }
  for(unsigned i = 0; i < ss.binvarCount; i++){
    if(s0.binvars[i] != s1.binvars[i]) return false;
  }
  return true;
}

//Structured Sample Collection functions

std::ostream& operator<<(std::ostream& o, const StructuredSampleCollection& s)
{
  writeSampleArr(o, s.samples, s.sampleStructure);
  return o;
}

std::ostream& operator<<(std::ostream& o, const StructuredSample& s)
{
  writeSample(o, s.sample, s.sampleStructure);
  return o;
}

//SampleType
bool SampleStructure::operator==(const SampleStructure &other) const {
  if(contvarCount != other.contvarCount || catvarCount != other.catvarCount || binvarCount != other.binvarCount) return false;
  for(unsigned i = 0; i < catvarCount; i++){
    if(catvarSizes[i] != other.catvarSizes[i]) return false;
    for(unsigned j = 0; j < catvarSizes[i]; j++){
      if(*catvarCatNameLookup(i, j) != *other.catvarCatNameLookup(i, j)){
        return false;
      }
    }
  }
  for(unsigned i = 0; i < binvarCount; i++){
    if(*binvarCatNameLookup(i, 0) != *other.binvarCatNameLookup(i, 0) || *binvarCatNameLookup(i, 1) != *other.binvarCatNameLookup(i, 1)) return false;
  }
  return true;
}

bool SampleStructure::operator!=(const SampleStructure &other) const {
  return !operator==(other);
}

////////
//SAMPLE


Array<contvar_t> Sample::contvarArray(SampleStructure st){
  return Array<contvar_t>(contvars, st.contvarCount);
}
Array<catvar_t> Sample::catvarArray(SampleStructure st){
  return Array<catvar_t>(catvars, st.catvarCount);
}
BitArray Sample::binvarArray(SampleStructure st){
  return binvars;
}

bool Sample::hasUnknowns(const SampleStructure& ss) const{
  #ifdef UNKNOWNVALUES
  for(unsigned shuffleIndex = 0; shuffleIndex < ss.contvarCount; shuffleIndex++){
    if(contvarUnknown(shuffleIndex)) return true;
  }
  for(unsigned shuffleIndex = 0; shuffleIndex < ss.catvarCount; shuffleIndex++){
    if(catvarUnknown(shuffleIndex)) return true;
  }
  for(unsigned shuffleIndex = 0; shuffleIndex < ss.binvarCount; shuffleIndex++){
    if(binvarUnknown(shuffleIndex)) return true;
  } 
  #endif
  return false;
}

unsigned Sample::countUnknowns(const SampleStructure& ss) const{
  #ifdef UNKNOWNVALUES
  unsigned count = 0;
  for(unsigned shuffleIndex = 0; shuffleIndex < ss.contvarCount; shuffleIndex++){
    if(contvarUnknown(shuffleIndex)) count++;
  }
  for(unsigned shuffleIndex = 0; shuffleIndex < ss.catvarCount; shuffleIndex++){
    if(catvarUnknown(shuffleIndex)) count++;
  }
  for(unsigned shuffleIndex = 0; shuffleIndex < ss.binvarCount; shuffleIndex++){
    if(binvarUnknown(shuffleIndex)) count++;
  }
  return count;
  #else
  return 0;
  #endif
}

//////////////////////////////
//STRUCTURED SAMPLE COLLECTION

contvar_t* StructuredSampleCollection::calculateMeanContFeatures() const{
  contvar_t* cvars = new contvar_t[sampleStructure.contvarCount];
  arrayZero<contvar_t>(cvars, sampleStructure.contvarCount); //Optimization: for a minimal numerics cost, could probably set to bitwise 0 (-0) faster.
  
  unsigned counts[sampleStructure.contvarCount];
  arrayZero<unsigned>(counts, sampleStructure.contvarCount);
  
  for(unsigned i = 0; i < samples.length; i++){
    for(unsigned j = 0; j < sampleStructure.contvarCount; j++){
      if(!isUnknownContVar(samples[i].contvars[j])){
        cvars[j] += samples[i].contvars[j];
        counts[j]++;
      }
    }
  }
  for(unsigned i = 0; i < sampleStructure.contvarCount; i++){
    if(counts[i] == 0){
      std::cerr << "Warning: Found no known data for continuous feature \"" << sampleStructure.contvarName(i) << "\" in mean calculation." << std::endl;
      cvars[i] = 0;
    }
    else cvars[i] /= counts[i];
  }
  
  return cvars;
}

contvar_t* StructuredSampleCollection::calculateStdevContFeatures(const contvar_t* means) const{
  contvar_t* cvars = new contvar_t[sampleStructure.contvarCount];
  arrayZero<contvar_t>(cvars, sampleStructure.contvarCount); //Optimization: for a minimal numerics cost, could probably set to bitwise 0 (-0) faster.

  unsigned counts[sampleStructure.contvarCount];
  arrayZero<unsigned>(counts, sampleStructure.contvarCount);

  for(unsigned i = 0; i < samples.length; i++){
    for(unsigned j = 0; j < sampleStructure.contvarCount; j++){
      if(!isUnknownContVar(samples[i].contvars[j])){
        cvars[j] += (samples[i].contvars[j] - means[j]) * (samples[i].contvars[j] - means[j]);
        counts[j]++;
      }
    }
  }
  for(unsigned i = 0; i < sampleStructure.contvarCount; i++){
    if(counts[i] == 0){
      std::cerr << "Warning: Found no known data for continuous feature \"" << sampleStructure.contvarName(i) << "\" in stdev calculation." << std::endl;
      cvars[i] = 1;
    }
    else cvars[i] = sqrt(cvars[i] / (counts[i] - 1)); //Apply Bessel's Correction and take sqrt
  }
  
  return cvars;
}



void StructuredSampleCollection::storeAverageContFeatures(){
  sampleStructure.contvarMeans = calculateMeanContFeatures();
}

//By the nature of the data stored in these objects, it is actually very difficult to validate them.  Certain properties, such as proper length arrays, cannot be checked.
bool StructuredSampleCollection::validate() const{
  //Make sure the catvar counts are valid.
  for(unsigned i = 0; i < sampleStructure.catvarCount; i++){
    if(sampleStructure.catvarSizes[i] > MAX_CAT){
      std::cerr << "Invalid catvar size: " << (unsigned)sampleStructure.catvarSizes[i] << " exceeds maximum of " << (unsigned)MAX_CAT << ".\n";
      return false;
    }
  }
  
  //Check each sample.
  for(unsigned i = 0; i < samples.length; i++){
    //Check that the Sample does not have out of bounds catvars.
    for(unsigned j = 0; j < sampleStructure.catvarCount; j++){
      //std::cout << "S " << i << ", " << j << ": " << (int)samples[i].catvars[j] << "/" << (int)sampleStructure.catvarSizes[j] << std::endl;
      if(samples[i].catvars[j] >= sampleStructure.catvarSizes[j] && samples[i].catvars[j] != UNKNOWN_CAT){
        std::cerr << "Sample " << i << " categorical feature " << j << " has value " << (unsigned)samples[i].catvars[j] << ".  The sample structure dictates that the max category for this feature is " << sampleStructure.catvarSizes[j] << ".\n";
        return false;
      }
    }
    
    //Check the lengths of binvars
    if(samples[i].binvars.size() != sampleStructure.binvarCount){
      std::cerr << "Sample " << i << "Invalid binvar array: contains " << samples[i].binvars.size() << " binvars, expected " << sampleStructure.binvarCount;
      return false;
    }
  }
  
  return true;
}
//Architecture: this would be a good way to get normalization.
/*
void StructuredSampleCollection::normalize() const{
  
}
*/
bool StructuredSampleCollection::hasUnknowns() const{
  for(unsigned i = 0; i < samples.length; i++){
    if(samples[i].hasUnknowns(sampleStructure)){
      return true;
    }
  }
  return false;
}
unsigned StructuredSampleCollection::countUnknowns() const{
  return samples.fold<unsigned, SampleStructure>(
    [](const unsigned acc, const Sample s, const SampleStructure ss){return acc + s.countUnknowns(ss);},
    0, sampleStructure);
}
