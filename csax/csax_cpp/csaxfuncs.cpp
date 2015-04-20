// Brett Fischler and Jacob Gerace
// April 2015
// Helper function implementations for CSAX algorithm

#include "csaxfuncs.h"
#include "genesetmanager.h"
#include "sample.h"
#include <vector>
#include <map>
#include <iostream>
using namespace std;

struct GeneScoreList {
    string gene;
    vector<float> scores;
};

// Declarations
vector<GeneScoreList> runFRaC(string traindata,
        string testdata, bool use_all);
vector<map<string, float>> runGSEA(string genesets_file,
        vector<GeneScoreList> genescores);
void CSAX_iteration(string traindata_file, string testdata_file,
        string genesets_file, int n_tests, vector<GeneSetManager*> managers);

// Definitions
void runCSAX(string traindata_file, string testdata_file,
        string genesets_file, int num_bags, float gamma)
{
    // First, we have to process the full training data
    vector<GeneScoreList> genescores = 
        runFRaC(traindata_file, testdata_file, true);

    // Run GSEA as well
    vector<map<string, float>> enrichmentscores =
        runGSEA(genesets_file, genescores);
    
    // Number of test instances
    int n_tests = genescores.size();

    vector<GeneSetManager*> managers;
    for (int i = 0; i < n_tests; i++) {
        managers.push_back(new GeneSetManager());
    }

    // For each bag run FRaC and GSEA
    for (int b = 0; b < num_bags; b++) {
        CSAX_iteration(traindata_file, testdata_file,
                genesets_file, n_tests, managers);

    }
    // TODO: Sort samples by median score:x
}

// Run FRaC and GSEA on random subset of traindata, and add ranking to each
// Sample in the traindata
void CSAX_iteration(string traindata_file, string testdata_file,
        string genesets_file, int n_tests, vector<GeneSetManager*> managers)
{
    // TODO: Somehow, specify random traindata instances to use

    // Run FRaC on sample of traindata
    vector<GeneScoreList> genescores = 
        runFRaC(traindata_file, testdata_file, false);

    // Run GSEA on output
    vector<map<string, float>> enrichmentscores =
        runGSEA(genesets_file, genescores);

    // For each test sample
    for (int i = 0; i < n_tests; i++) {
        for (std::map<string, float>::iterator it=enrichmentscores[i].begin();
                it != enrichmentscores[i].end(); it++) {
            managers[i]->addRankingToGeneset(it->second, it->first);
        }
    }
}

vector<GeneScoreList> runFRaC(string traindata_file,
        string testdata_file, bool use_all)
{
    // TODO: Call FRaC program, which takes in traindata and testdata
    // and returns a table of anomaly scores, where the rows represent
    // genes and the columns represent test instances.
    vector<GeneScoreList> genescores;
    return genescores;
}

vector<map<string, float>> runGSEA(string genesets_file,
        vector<GeneScoreList> genescores)
{
    //TODO: Call GSEA program, which takes in a gene set database and genescores
    // and returns a list of genes, ranked by their "enrichment"
    vector<map<string, float>> enrichment_scores;
    return enrichment_scores;
}
