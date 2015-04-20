// Brett Fischler and Jacob Gerace
// April 2015
// Helper function declarations for the CSAX algorithm

#include "sample.h"
using namespace std;

struct GeneScorePair;

void runCSAX(string traindata_file, string testdata_file,
        string genesets_file, int num_bags, float gamma);
