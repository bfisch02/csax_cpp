// Brett Fischler and Jacob Gerace
// April 2015
// Helper function declarations for the CSAX algorithm

#include "sample.h"
#include <vector>
#include <iostream>
using namespace std;

struct GeneScorePair;

struct SampleList {
    vector<Sample *> data;
    vector<string> geneNames;
};

/*only one public function, see class for details */
void runCSAX(SampleList traindata, SampleList testdata, string testfile,
        string genesets_file, string output_dir, int num_bags, double gamma);
