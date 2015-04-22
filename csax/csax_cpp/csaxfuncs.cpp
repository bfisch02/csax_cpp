// Brett Fischler and Jacob Gerace
// April 2015
// Helper function implementations for CSAX algorithm

#include "csaxfuncs.h"
//#include <boost/algorithm/string/split.hpp>
#include <algorithm>
#include "genesetmanager.h"
#include "sample.h"
#include <vector>
#include <map>
#include <fstream>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <cstring>
using namespace std;

const string FRAC_OUTPUT = "ns.gz";

struct GeneScoreList {
    string gene;
    vector<double> scores;
};

// Declarations
vector<GeneScoreList *> runFRaC(SampleList traindata, string testfile,
        bool use_all);
vector<map<string, double>*> runGSEA(string genesets_file,
        vector<GeneScoreList *> genescores);
void CSAX_iteration(SampleList traindata, SampleList testdata, string testfile,
        string genesets_file, vector<GeneSetManager *> managers);
void samplesToFile(SampleList samples, string filename, double percent_to_add);
vector<GeneScoreList *> parseFRaCOutput(string fracFileP);

// Definitions

void runCSAX(SampleList traindata, SampleList testdata, string testfile,
        string genesets_file, int num_bags, double gamma)
{
    // First, we have to process the full training data
    vector<GeneScoreList *> genescores =
        runFRaC(traindata, testfile, true);

    // Run GSEA as well
    vector<map<string, double>*> ES = runGSEA(genesets_file, genescores);

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

    double cur_score = 0;
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

    // Run FRaC on sample of traindata
    vector<GeneScoreList *> genescores =
        runFRaC(traindata, testfile, false);

    //UP TO THIS POINT WOOHOO

    // Run GSEA on output
    vector<map<string, double> *> enrichmentscores =
        runGSEA(genesets_file, genescores);

    // For each test sample
    for (unsigned i = 0; i < testdata.data.size(); i++) {
        for (std::map<string, double>::iterator it=enrichmentscores[i]->begin();
                it != enrichmentscores[i]->end(); it++) {
            managers[i]->addRankingToGeneset(it->second, it->first);
        }
    }
}

vector<GeneScoreList *> runFRaC(SampleList traindata, string testfile,
        bool use_all)
{
    /*
    (void)testfile;
    string trainfile = "trainfile";
    if (use_all) {
        samplesToFile(traindata, trainfile, 1);
    } else {
        samplesToFile(traindata, trainfile, .5);
    }
    cerr << "the testfile is: " << testfile << endl;
    */
    // TODO: Call FRaC program, which takes in traindata and testdata
    // and returns a table of anomaly scores, where the rows represent
    // genes and the columns represent test instances.


    //string command = "../csax_r/frac.r " + trainfile + " ../csax_r/examples.input/example.test.set FRaC_gene_anomaly_buffer";
    //string command = "./frac.r " + trainfile + " " + testfile + " FRaC_anomaly_buffer";
    //system(command.c_str());


    cerr << "here" << endl;
    string fracFileP = "output_location/ns.gz";
    ifstream matrix;
    matrix.open("output_location/ns.gz");
    double name;
    matrix >> name;
    cerr << "First name is: " << name;
    matrix >> name;
    cerr << "First name is: " << name;


    //remove(fracFileP.c_str());
    cerr << "here2" << endl;

    exit(0);
    vector<GeneScoreList *> genescores = parseFRaCOutput(fracFileP);
    //for right now, printing out the file we got back
    /*
    for (unsigned i = 0; i < genescores.size(); i++) {
        cout << genescores[i]->gene;
        unsigned numQuery = genescores[i]->scores.size();
        for (unsigned j = 0; j < numQuery; j++) {
            cout << "\t" << genescores[i]->scores[j];
        }
        cout << endl;
    }
    cerr << "here3" << endl;
    exit(0);
    */
    return genescores;
}


//struct GeneScoreList {
//    string gene;
//    vector<double> scores;
//};
vector<GeneScoreList *> parseFRaCOutput(string fracFileP)
{
    cerr << "go west" << endl;
    vector<GeneScoreList *> inputBuffer;
    ifstream matrix;
    matrix.open(fracFileP);
    string line = "";
    string name;

    vector<string> nameGenes;
    double tempdouble = 0.0;
    unsigned numNames = 0;
    GeneScoreList * p;
    cerr << "life is peaceful there" << endl;
    if (matrix.is_open()) {
        //Don't need the header, just call getline to chuck it
        cerr << "go west2" << endl;
        getline(matrix, line);
        istringstream lineStream (line);
        while (lineStream >> name) {
            cerr << name;
            numNames++;
         }
        cerr << "in the openair" << endl;
        while (matrix >> name) {
            //nameGenes.push_back(name);
            cerr << "where the skies are blue" << endl;
            vector<double> oneRow;
            for (unsigned i = 0; i < numNames; i++) {
                cerr << "this is what we'll do" << endl;
                matrix >> tempdouble;
                oneRow.push_back(tempdouble);
                cerr << tempdouble << endl;
            }
            cerr << "sun in wintertime" << endl;
            p = new GeneScoreList;
            p->gene = name;
            p->scores = oneRow;
            cerr << "we will be just fine" << endl;

            inputBuffer.push_back(p);
        }
    }
    cerr << "go west" << endl;
    return inputBuffer;
}

vector<map<string, double>*> runGSEA(string genesets_file,
        vector<GeneScoreList *> genescores)
{
    (void)genesets_file;
    (void)genescores;
    //TODO: Call GSEA program, which takes in a gene set database and genescores
    // and returns a list of genes, ranked by their "enrichment"
    vector<map<string, double>*> enrichment_scores;
    return enrichment_scores;
}

void samplesToFile(SampleList samples, string filename, double percent_to_add)
{
    ofstream f;
    double fractionBag = percent_to_add;
    unsigned numTraining = samples.data.size();
    unsigned numTrue = fractionBag * numTraining;
    vector<bool> truthVector;

    cerr << "writing training data to: " << filename << endl;
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
