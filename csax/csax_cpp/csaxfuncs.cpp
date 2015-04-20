// Brett Fischler and Jacob Gerace
// April 2015
// Helper function implementations for CSAX algorithm

#include "csaxfuncs.h"
#include "sample.h"
#include <iostream>
using namespace std;

struct GeneScorePair {
    string gene;
    float score;
};

// DECLARATIONS

vector<GeneScorePair> runFRaC(vector<Sample> traindata,
        vector<Sample> testdata);
vector<float> runGSEA(vector<Sample> traindata, vector<GeneScorePair> gscores);
void CSAX_iteration(vector<Sample> traindata, vector<Sample> testdata);

// DEFINITIONS

void runCSAX(vector<Sample> traindata, vector<Sample> testdata, int numBags)
{
    // First, process full training data
    vector<GeneScorePair> genescores = runFRaC(traindata, testdata);

    // Run GSEA as well
    vector<float> enrichmentscores = runGSEA(traindata, genescores);

    // Run bagging iterations
    for (int b = 0; bag < numBags; b++) {
        CSAX_iteration(traindata, testdata);
    }

    // TODO: Sort samples by median score:x
}

// Run FRaC and GSEA on random subset of traindata, and add ranking to each
// Sample in the traindata
void CSAX_iteration(vector<Sample> traindata, vector<Sample> testdata)
{
    // The indices of our random Samples for this iteration
    // TODO: Actually get these
    vector<int> selected_indices;

    // Subset of trainda
    // TODO: Get random subset
    vector<Sample> traindata_subset;

    // Run FRaC and GSEA
    vector<GeneScorePair> genescores = runFRaC(traindata_subset, testdata);
    vector<float> enrichmentscores = runGSEA(traindata_subset, genescores);

    //TODO: For each Sample in traindata_subset, add ranking to list
}

vector<GeneScorePair> runFRaC(vector<Sample> traindata,
        vector<Sample> testdata)
{
    // TODO: Call FRaC program, which takes in traindata and testdata
    // and returns a table of anomaly scores for genes and for each
    // test instance. For this program, we are only interested in the
    // anomaly scores for the genes.
    vector<GeneScorePair> genescores;
    return genescores;
}

vector<float> runGSEA(vector<Sample> traindata, vector<GeneScorePair> gscores)
{
    //TODO: Call GSEA program, which takes in traindata and genescores and
    // returns a list of genes, ranked by their "enrichment"
    vector<float> enrichment_scores;
    return enrichment_scores;
}
