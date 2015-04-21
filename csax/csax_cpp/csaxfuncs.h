// Brett Fischler and Jacob Gerace
// April 2015
// Helper function declarations for the CSAX algorithm

#include "sample.h"
#include <vector>
#include <iostream>
using namespace std;

struct GeneScorePair;

void runCSAX(vector<Sample *> traindata, vector<Sample *> testdata,
        string genesets_file, vector<string> genenames, int num_bags,
        float gamma);