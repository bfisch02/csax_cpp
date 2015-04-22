#ifndef SAMPLE_H
#define SAMPLE_H

#include <inttypes.h>
#include <string>
#include <ostream>

#include "array.hpp"
#include "types.h"


////////////////////////////////////
//REPRESENTATIONS OF SAMPLE VALUES//
////////////////////////////////////

//CONTINUOUS
typedef fracfloat_t contvar_t;
#define UNKNOWN_CONT std::numeric_limits<contvar_t>::signaling_NaN()


#if defined __FAST_MATH__ || !defined isnan
//Adapted from http://stackoverflow.com/questions/17614943/good-sentinel-value-for-double-if-prefer-to-use-ffast-math
static inline int isUnknownContVar(float f)
{
  union { float f; uint32_t x; } u = { f };
  return (u.x << 1) > 0xff000000u;
}
static inline int isUnknownContVar(double d)
{
  union { double d; uint64_t x; } u = { d };
  return (u.x << 1) == 0xff70000000000000ull;
}
#else
#define isUnknownContVar(a) std::isnan(a)
#endif


//CATEGORICAL
#ifdef CAT_16_BIT

//16 bit categorical values
typedef uint16_t catvar_t;
#define MAX_CAT 65534
#define UNKNOWN_CAT 65535

#else

//8 bit categorical values
typedef uint8_t catvar_t;
#define MAX_CAT 254
#define UNKNOWN_CAT 255

#endif

#define isUnknownCatVar(a) (a == UNKNOWN_CAT)

//BINARY
typedef bool binvar_t;

#define BINVAR0 false;
#define BINVAR1 true;

#include <iostream>

class SampleStructure{
  
  private:
  //Making this data private is more trouble than its worth.
  public:  
  unsigned contvarCount, catvarCount, binvarCount;
  unsigned* catvarSizes;
  std::string** catvarCatNames;
  std::string* binvarCatNames;
  std::string* nameLookup;
  
  std::string name;
  
  //Architecture: contvarMeans doesn't belong in SampleStructure.
  contvar_t* contvarMeans;

  public:
  SampleStructure() : contvarMeans(NULL) {} //For array construction.
  SampleStructure(unsigned contvarCount, unsigned catvarCount, unsigned binvarCount, unsigned* catvarSizes, std::string* nameLookup, std::string name) : contvarCount(contvarCount), catvarCount(catvarCount), binvarCount(binvarCount), catvarSizes(catvarSizes), nameLookup(nameLookup), name(name), contvarMeans(NULL){
    catvarCatNames = new std::string*[catvarCount];
    //Consider consolidating this into a single block.
    for(unsigned i = 0; i < catvarCount; i++){
      catvarCatNames[i] = new std::string[catvarSizes[i]];
    }
    binvarCatNames = new std::string[binvarCount * 2];
  }
  
  SampleStructure(unsigned contvarCount, unsigned catvarCount, unsigned binvarCount, unsigned* catvarSizes, std::string** catvarCatNames, std::string* binvarCatNames, std::string* nameLookup, std::string name) : contvarCount(contvarCount), catvarCount(catvarCount), binvarCount(binvarCount), catvarSizes(catvarSizes), catvarCatNames(catvarCatNames), binvarCatNames(binvarCatNames), nameLookup(nameLookup), name(name), contvarMeans(NULL) { }

  //Index lookup functions.  These look up the absolute index of a feature.
  unsigned contvarIndex(unsigned index) const {
    assert(index < contvarCount);
    return index;
  }
  unsigned catvarIndex(unsigned index) const {
    assert(index < catvarCount);
    return index + contvarCount;
  }
  unsigned binvarIndex(unsigned index) const {
    assert(index < binvarCount);
    return index + contvarCount + catvarCount;
  }
  
  
  //Name pointer lookups
  std::string* contvarNamePtr(unsigned index) const {
    assert(index < contvarCount);
    return nameLookup + contvarIndex(index);
  }
  std::string* catvarNamePtr(unsigned index) const {
    assert(index < catvarCount);
    return nameLookup + catvarIndex(index);
  }
  std::string* binvarNamePtr(unsigned index) const {
    assert(index < binvarCount);
    return nameLookup + binvarIndex(index);
  }
  
  
  //Name lookup functions:
  std::string contvarName(unsigned index) const {
    assert(index < contvarCount);
    return *contvarNamePtr(index);
  }
  std::string catvarName(unsigned index) const {
    assert(index < catvarCount);
    return *catvarNamePtr(index);
  }
  std::string binvarName(unsigned index) const {
    assert(index < binvarCount);
    return *binvarNamePtr(index);
  }
  
  std::string featureName(unsigned index) const {
    assert(index < featureCount());
    return nameLookup[index];
  }
  
  //Category name lookups
  std::string* catvarCatNameLookup(unsigned catFeatureIndex, catvar_t cv) const {
    assert(cv < catvarSizes[catFeatureIndex]);
    return &catvarCatNames[catFeatureIndex][cv];
  }
  std::string* binvarCatNameLookup(unsigned binFeatureIndex, binvar_t bv) const {
      assert(bv == 0 || bv == 1);
    return &binvarCatNames[binFeatureIndex * 2 + bv];
  }
  
  unsigned featureCount() const { return contvarCount + catvarCount + binvarCount; }

  friend std::ostream& operator<<(std::ostream& o, const SampleStructure& s);
  
  //Equality test:
  bool operator==(const SampleStructure &other) const;
  bool operator!=(const SampleStructure &other) const;
  
  //Quality: rename freeMemory for consistency.
  void freeData(){
    delete [] catvarSizes;
    
    delete [] nameLookup;
    
    for(unsigned i = 0; i < catvarCount; i++){
      delete [] catvarCatNames[i];
    }
    
    delete [] catvarCatNames;
    delete [] binvarCatNames;
    
    if(contvarMeans != 0) delete [] contvarMeans;
  }
};

//A sample is a collection of values that can be interpreted in the context of a SampleStructure
class Sample{
  public:
  contvar_t* contvars;
  catvar_t* catvars;
  BitArray binvars;
  
  std::vector<unsigned> binvarunknowns; //This is a bit of a mixed representation, the bit array can't express unknowns, so they are indexed here.  Sometimes the most efficient representation is also the most painful.
  
  //Architecture: Consider abstracting into a NullableArray<Array<T>> class NullableArray<BitArray>.  It may be more efficient to represent nulls as a list?
  
  //This is to satisfy some strange quirk of lambdas and the type system; using indexing on the bit array causes compilation to fail but this function works.
  binvar_t getBinvar(unsigned index){
    return binvars[index];
  }
  
  
  //////////////
  //Constructors
  
  Sample(){} //For arrays.  There might be a more idiomatic way of expressing this.
  
  Sample(contvar_t* contvars, catvar_t* catvars, BitArray binvars, std::vector<unsigned> binvarunknowns){
    this->contvars = contvars;
    this->catvars = catvars;
    this->binvars = binvars;
    this->binvarunknowns = binvarunknowns;
  }
  
  Sample(unsigned contvarCount, unsigned catvarCount, unsigned binvarCount){
    contvars = new contvar_t[contvarCount]; 
    catvars =  new catvar_t[catvarCount];
    binvars =  BitArray(binvarCount);
  }
  
  Sample(SampleStructure& st) : Sample(st.contvarCount, st.catvarCount, st.binvarCount) {}
  
  //////////////////
  //Query Operations
  
  bool contvarUnknown(unsigned index) const{
    #ifdef UNKNOWNVALUES
    return isUnknownContVar(contvars[index]);
    #else
    return false;
    #endif
  }
  
  bool catvarUnknown(unsigned index) const{
    #ifdef UNKNOWNVALUES
    return isUnknownCatVar(catvars[index]);
    #else
    return false;
    #endif
  }
  
  bool binvarUnknown(unsigned index) const{
    #ifdef UNKNOWNVALUES
    return std::binary_search(binvarunknowns.begin(), binvarunknowns.end(), index);
    #else
    return false;
    #endif
  }
  
  
  bool hasUnknowns(const SampleStructure& ss) const;
  unsigned countUnknowns(const SampleStructure& ss) const;
  
  Array<contvar_t> contvarArray(SampleStructure st);
  Array<catvar_t> catvarArray(SampleStructure st);
  BitArray binvarArray(SampleStructure st);
  
  
  void freeMemory(){
    delete [] contvars;
    delete [] catvars;
  }
};

void writeSample(std::ostream& out, const Sample& sample, const SampleStructure& st);
void writeSampleArr(std::ostream& out, const Array<Sample>& samples, const SampleStructure& st);

bool sampleEq(const SampleStructure ss, const Sample s0, const Sample s1);

class StructuredSampleCollection{
  public:
  SampleStructure sampleStructure;
  Array<Sample> samples;
  
  public:
  
  StructuredSampleCollection(){ }
  StructuredSampleCollection(SampleStructure sampleStructure, Array<Sample> samples) : sampleStructure(sampleStructure), samples(samples) {
    assert(validate());
  }
  
  contvar_t* calculateMeanContFeatures() const;
  contvar_t* calculateStdevContFeatures(const contvar_t* means) const;

  Sample calculateAverageSample() const;
  void storeAverageContFeatures();

  bool validate() const;
  void normalize();
  bool hasUnknowns() const;
  unsigned countUnknowns() const;
  
  fracfloat_t maxContvar(unsigned index){
    assert(samples.length > 0);
    assert(index < sampleStructure.contvarCount);
    unsigned max = samples[0].contvars[index];
    for(unsigned i = 1; i < samples.length; i++){
      if(max < samples[i].contvars[index]){ //This ordering is to catch NaNs.
        max = samples[i].contvars[index];
      }
    }
    return max;
  }
  
  fracfloat_t minContvar(unsigned index){
    assert(samples.length > 0);
    assert(index < sampleStructure.contvarCount);
    unsigned min = samples[0].contvars[index];
    for(unsigned i = 1; i < samples.length; i++){
      if(min > samples[i].contvars[index]){ //This ordering is to catch NaNs.
        min = samples[i].contvars[index];
      }
    }
    return min;
  }
  
  friend std::ostream& operator<<(std::ostream& o, const StructuredSampleCollection& s);
  
  void freeMemory(){
    samples.forEach([](Sample& s){s.freeMemory();});
    samples.freeMemory();
    sampleStructure.freeData();
  }
  
  bool operator==(StructuredSampleCollection other) const {
    if (samples.length != other.samples.length || sampleStructure != other.sampleStructure) return false;
    
    for(unsigned i = 0; i < samples.length; i++){
      if(!sampleEq(sampleStructure, samples[i], other.samples[i])) return false;
    }
    
    return true;
  }
  
  bool operator!=(StructuredSampleCollection other) const {
    return !(this->operator==(other));
  }
};

struct StructuredSample{
  SampleStructure sampleStructure;
  Sample sample;
    
  StructuredSample(SampleStructure st, Sample s) : sampleStructure(st), sample(s) { }

  friend std::ostream& operator<<(std::ostream& o, const StructuredSample& s);    
};

#endif
