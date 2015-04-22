#ifndef IO_H
#define	IO_H

#include <iostream>
#include <fstream>

#include "sample.hpp"
#include "frac.hpp"

#include <vector>

///////
//Types
enum featurety_t{ feature_contvar, feature_catvar, feature_ignore };
typedef std::vector<featurety_t> FeatureOrdering;


/////////////////
//File IO helpers

void openOutputFile(std::ofstream& s, std::string path);
void openInputFile(std::ifstream& s, std::string path);


//////////////////
//String Functions

std::vector<std::string> split(const std::string& s, const std::string& delim, const bool keep_empty = true);

// trim string
std::string &trim(std::string &s);

void tolower(std::string& s);

//Used to sanitize some outputs
std::string spaceToUnderscore(std::string s);

//Error output function
std::ostream& parserError(unsigned lineNum);

/////////
//Reading

StructuredSampleCollection readData(std::string split, std::istream& in, unsigned lineNum, SampleStructure st, FeatureOrdering& featureTypes);

StructuredSampleCollection readArff(std::string fileName);
StructuredSampleCollection readArff(std::istream& in);

/////////
//Writing

void writeArff(std::ostream& out, StructuredSampleCollection ssc);
void writeArff(std::ostream& out, SampleStructure structure, Array<SampleScorePair> scoredSamples);

#endif	/* IO_HPP */

