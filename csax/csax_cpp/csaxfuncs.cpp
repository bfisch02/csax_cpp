// Brett Fischler and Jacob Gerace
// April 2015
// Helper function implementations for CSAX algorithm

#include "csaxfuncs.h"
#include <boost/algorithm/string/split.hpp>
#include "genesetmanager.h"
#include "sample.h"
#include <vector>
#include <map>
#include <fstream>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
using namespace std;

const string FRAC_OUTPUT = "ns.gz";

struct GeneScoreList {
    string gene;
    vector<float> scores;
};

// Declarations

vector<GeneScoreList *> runFRaC(SampleList traindata, string testfile,
        bool use_all);
vector<map<string, float>*> runGSEA(string genesets_file,
        vector<GeneScoreList *> genescores);
void CSAX_iteration(SampleList traindata, SampleList testdata, string testfile,
        string genesets_file, vector<GeneSetManager *> managers);
void samplesToFile(SampleList samples, string filename, double percent_to_add);

// Definitions

void runCSAX(SampleList traindata, SampleList testdata, string testfile,
        string genesets_file, int num_bags, float gamma)
{
    // First, we have to process the full training data
    vector<GeneScoreList *> genescores =
        runFRaC(traindata, testfile, true);

    // Run GSEA as well
    vector<map<string, float>*> ES = runGSEA(genesets_file, genescores);

    vector<GeneSetManager*> managers;
    for (unsigned i = 0; i < testdata.data.size(); i++) {
        managers.push_back(new GeneSetManager());
    }

    for (int b = 0; b < num_bags; b++) {
        CSAX_iteration(traindata, testdata, testfile, genesets_file, managers);
    }

    cout << "STILL GOOD" << endl;
    exit(1);

    for (unsigned i = 0; i < managers.size(); i++) {
        managers[i]->sortByMedian();
    }

    float cur_score = 0;
    for (unsigned i = 0;i  < testdata.data.size(); i++) {
        cur_score = managers[i]->getAnomalyScore(gamma, ES[i]);
        cout << "Score for " << i << ": " << cur_score << endl;
    }

}

// Run FRaC and GSEA on random subset of traindata, and add ranking to each
// Sample in the traindata
void CSAX_iteration(SampleList traindata, SampleList testdata, string testfile,
        string genesets_file, vector<GeneSetManager *> managers)
{
    // TODO: Somehow, specify random traindata instances to use

    // Run FRaC on sample of traindata
    vector<GeneScoreList *> genescores = 
        runFRaC(traindata, testfile, false);

    // Run GSEA on output
    vector<map<string, float> *> enrichmentscores =
        runGSEA(genesets_file, genescores);

    // For each test sample
    for (unsigned i = 0; i < testdata.data.size(); i++) {
        for (std::map<string, float>::iterator it=enrichmentscores[i]->begin();
                it != enrichmentscores[i]->end(); it++) {
            managers[i]->addRankingToGeneset(it->second, it->first);
        }
    }
}

vector<GeneScoreList *> runFRaC(SampleList traindata, string testfile,
        bool use_all)
{
    (void)testfile;
    string trainfile = "trainfile";
    if (use_all) {
        samplesToFile(traindata, trainfile, 1);
    } else {
        samplesToFile(traindata, trainfile, .5);
    }
    // TODO: Call FRaC program, which takes in traindata and testdata
    // and returns a table of anomaly scores, where the rows represent
    // genes and the columns represent test instances.

    // system("./frac.r " + trainfile + " " + testfile = " " + FRAC_OUTPUT)

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

void samplesToFile(SampleList samples, string filename, double percent_to_add)
{
    ofstream f;
    double fractionBag = percent_to_add;
    unsigned numTraining = samples.data.size();
    unsigned numTrue = fractionBag * numTraining;
    vector<bool> truthVector;

    f.open(filename);

    //make a random permutation parallel array
    for (unsigned i = 0; i < numTrue; i++) {
        truthVector.push_back(true);
    }
    for (unsigned i = numTrue; i < numTraining; i++) {
        truthVector.push_back(false);
    }
    std::random_shuffle (truthVector.begin(), truthVector.end());

    //output training data to a file
    for (unsigned i = 0; i < samples.data.size(); i++) {
        if (truthVector[i]) {
            f << samples.data[i]->getName() << "\t";
        }
    }
    f << endl;

    unsigned numGenes = samples.geneNames.size();
    for (unsigned j = 0; j < numGenes; j++) {
        f << samples.geneNames[j] << "\t";
        for (unsigned i = 0; i < samples.data.size(); i++) {
            if (truthVector[i])
                f << samples.data[i]->getGene(j) << "\t";
        }
        f << endl;
    }

    f.close();
}
