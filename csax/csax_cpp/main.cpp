// Brett Fischler and Jacob Gerace
// April 2015
// Sample Class Implementation

#include "sample.h"
#include "csaxfuncs.h"
#include <stdlib.h>
#include <stdio.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
//#include <boost/algorithm/string/split.hpp>
using namespace std;
/* CSAX commandline can be run similar to
# CSAX
echo "Run CSAX:"
# (run bagging iterations 1 through 5 only)
csax.r -B 5 examples.input/example.training.set examples.input/example.test.set examples.input/reactome.gmt examples.output/csax examples.output/csax.g
echo -ne "\nRESULT: CSAX AUC=`auc.r examples.input/example.test.set.labels examples.output/csax.g`\n\n"
 *from the examples.bash file
 */
//struct SampleList {
 //   vector<Sample *> data;
  //  vector<string> geneNames;
//};
SampleList getData(string matrixFile);
void printSamples(vector<Sample> samples);

int main(int argc, char **argv) {
    if (argc < 5) {
        cerr << "Usage: ./csax [options] <training set> <test set> "
             << "<gene set database> <output directory>"
             << endl;
        exit(1);
    }

    int num_bags = 40; // Default number of bags
    double gamma = .95; // Default gamma value

    SampleList traindata = getData(argv[1]);
    SampleList testdata = getData(argv[2]);
    runCSAX(traindata, testdata, (string)argv[2], (string)argv[3], (string)argv[4], num_bags, gamma);

    // TODO: Add options parsing
}

SampleList getData(string matrixFile)
{
    vector<Sample*> inputBuffer;
    ifstream matrix;
    matrix.open(matrixFile);
    string line = "";
    string name;
    vector<string> nameSamples;
    vector<string> nameGenes;
    unsigned numNames = 0;
    double tempdouble = 0.0;
    vector<vector<double>> buffer;
    if (matrix.is_open()) {
        getline(matrix, line);
        istringstream lineStream (line);
        while (lineStream >> name) {
            nameSamples.push_back(name);
            numNames++;
        }
        while (matrix >> name) {
            nameGenes.push_back(name);
            vector<double> oneRow;

            for (unsigned i = 0; i < numNames; i++) {
                matrix >> tempdouble;
                oneRow.push_back(tempdouble);
            }
            buffer.push_back(oneRow);
        }
        unsigned numGenes = nameGenes.size();
        //using buffer, extract one sample at a time, put it into the return
        //vector
        for (unsigned j = 0; j < numNames; j++) {
            vector<double> onesample;
            for (unsigned i = 0; i < buffer.size(); i++) {
                onesample.push_back(buffer[i][j]);
            }
            Sample *newSample = new Sample(numGenes, (string)nameSamples[j], onesample);
            inputBuffer.push_back(newSample);
        }
    }

    return {inputBuffer, nameGenes};
}

void printSamples(vector<Sample> samples)
{
    int j = 0;
    for (auto i = samples.begin(); i != samples.end(); i++) {
        cout << "The " << j << "th" << "sample is: " << endl;
        (*i).print();
        cout << endl;
        j++;
    }
}

