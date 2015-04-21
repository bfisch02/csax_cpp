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

vector<GeneScoreList *> runFRaC(vector<Sample *> traindata,
        vector<Sample *> testdata, vector<string> genenames, bool use_all);
vector<map<string, float>*> runGSEA(string genesets_file,
        vector<GeneScoreList *> genescores);
void CSAX_iteration(vector<Sample*> traindata, vector<Sample*> testdata,
        string genesets_file, vector<string> genenames,
        vector<GeneSetManager*> managers);

// Definitions

void runCSAX(vector<Sample *> traindata, vector<Sample *> testdata,
        string genesets_file, vector<string> genenames, int num_bags,
        float gamma)
{
    // First, we have to process the full training data
    vector<GeneScoreList *> genescores =
        runFRaC(traindata, testdata, genenames, true);

    // Run GSEA as well
    vector<map<string, float>*> ES = runGSEA(genesets_file, genescores);

    vector<GeneSetManager*> managers;
    for (unsigned i = 0; i < testdata.size(); i++) {
        managers.push_back(new GeneSetManager());
    }

    for (int b = 0; b < num_bags; b++) {
        CSAX_iteration(traindata, testdata, genesets_file, genenames, managers);
    }

    for (unsigned i = 0; i < managers.size(); i++) {
        managers[i]->sortByMedian();
    }

    float cur_score = 0;
    for (unsigned i = 0;i  < testdata.size(); i++) {
        cur_score = managers[i]->getAnomalyScore(gamma, ES[i]);
        cout << "Score for " << i << ": " << cur_score << endl;
    }

}

// Run FRaC and GSEA on random subset of traindata, and add ranking to each
// Sample in the traindata
void CSAX_iteration(vector<Sample*> traindata, vector<Sample*> testdata,
        string genesets_file, vector<string> genenames,
        vector<GeneSetManager*> managers)
{
    // TODO: Somehow, specify random traindata instances to use

    // Run FRaC on sample of traindata
    vector<GeneScoreList *> genescores = 
        runFRaC(traindata, testdata, genenames, false);

    // Run GSEA on output
    vector<map<string, float> *> enrichmentscores =
        runGSEA(genesets_file, genescores);

    // For each test sample
    for (unsigned i = 0; i < testdata.size(); i++) {
        for (std::map<string, float>::iterator it=enrichmentscores[i]->begin();
                it != enrichmentscores[i]->end(); it++) {
            managers[i]->addRankingToGeneset(it->second, it->first);
        }
    }
}

vector<GeneScoreList *> runFRaC(vector<Sample *> traindata,
        vector<Sample *> testdata, vector<string> genenames, bool use_all)
{
    (void)traindata;
    (void)testdata;
    (void)genenames;
    (void)use_all;
    // TODO: Call FRaC program, which takes in traindata and testdata
    // and returns a table of anomaly scores, where the rows represent
    // genes and the columns represent test instances.
    vector<GeneScoreList *> genescores;
    return genescores;
}

vector<map<string, float>*> runGSEA(string genesets_file,
        vector<GeneScoreList *> genescores)
{
    (void)genesets_file;
    (void)genescores;
    //TODO: Call GSEA program, which takes in a gene set database and genescores
    // and returns a list of genes, ranked by their "enrichment"
    vector<map<string, float>*> enrichment_scores;
    return enrichment_scores;
}
