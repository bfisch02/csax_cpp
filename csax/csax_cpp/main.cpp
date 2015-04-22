// Brett Fischler
// April 2015
// Sample Class Implementation
//
// Modifications made by Jacob Gerace

#include "sample.h"
#include "csaxfuncs.h"
#include <stdlib.h>
#include <stdio.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <boost/algorithm/string/split.hpp>
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
             << "<gene set database> <cache directory> [<output file>]"
             << endl;
        exit(1);
    }

    int num_bags = 40; // Default number of bags
    double gamma = .95; // Default gamma value
    (void)num_bags;
    (void)gamma;


    SampleList traindata = getData(argv[1]);
    SampleList testdata = getData(argv[2]);
    runCSAX(traindata, testdata, argv[2], argv[3], 40, .95);

    /*ofstream test;
    test.open("testdata_buffer");

    //output testdata to a file
    for (unsigned i = 0; i < testdata.data.size(); i++) {
        test << testdata.data[i]->getName() << "\t";
    }
    test << endl;

    unsigned numGenes = testdata.geneNames.size();
    for (unsigned j = 0; j < numGenes; j++) {
        test << testdata.geneNames[j] << "\t";
        for (unsigned i = 0; i < testdata.data.size(); i++) {
            test << traindata.data[i]->getGene(j) << "\t";
        }
        test << endl;
    }

    test.close();*/

    //system("./bashtest.bash");
    // TODO: Add options parsing

    //runCSAX(argv[3], argv[4], argv[5], num_bags, gamma);
}

SampleList getData(string matrixFile)
{
    //cerr << "Beginning of getData" << endl;
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
    //Sample newSample;
    //vector<vector<double>> inputBuffer;
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
            //Sample newSample(numGenes, onesample);
            Sample *newSample = new Sample(numGenes, nameSamples[j], onesample);
            inputBuffer.push_back(newSample);
        }
    }

    //ifstream testoutput;
    //testoutput.open("outputfilename");
    //for (int i = 0; i < inputBuf
    //matrix.close();
    //for (unsigned i = 0; i < inputBuffer.size(); i++) {
    //    cout << inputBuffer[i] << "\t";
   // }
    //cout << endl;

    //code to reproduce input file
    //for (in
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

//My main.cpp from hwk5, for reference
////main.cpp
//Jacob Gerace
/*
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <vector>
#include "classifier.h"
#include <algorithm>
using namespace std;

int main(int argc, char* argv[])
{
        if (argc < 5) {
                cout << "Input format incorrect" << endl;
                cout << "Format is: ./classifer <training-data-file>"
                     << " <data-file> <k-nearest-neighbors>"
                     << " <num-genes-to use>" << endl;
                return 1;
        }
        //read in arguments from command line
        int numGenes = 0;
        int numClassZero = 0;
        int numClassOne = 0;
        int kValue      = atoi(argv[3]);
        int numGenesUse = atoi(argv[4]);

        ifstream training_stream;
        ifstream query_stream;

        training_stream.open(argv[1]);
        query_stream.open(argv[2]);

        //read in header in file
        training_stream >> numGenes;
        training_stream >> numClassZero;
        training_stream >> numClassOne;

        Classifier MagicBall(kValue, numGenes, numGenesUse);

        double tempVal = 0.0;
        vector<vector<double>> enteredData;

        //read in data to a buffer
        for (int i = 0; i < numGenes; i++) {
                vector<double> oneRow;
                for (int j = 0; j < (numClassZero + numClassOne); j++) {
                        training_stream >> tempVal;
                        oneRow.push_back(tempVal);
                }
                enteredData.push_back(oneRow);
        }

        //get just the gene data for one zero sample
        //add each one, one at a time
        for (int i = 0; i < numClassZero; i++) {
                vector<double> onesample;
                for (int j = 0; j < numGenes; j++) {
                        onesample.push_back(enteredData[j][i]);
                }
                MagicBall.add_sample(onesample, 0);
        }
        //same process, now just adding the one samples
        for (int i = numClassZero; i < (numClassZero + numClassOne); i++) {
                vector<double> onesample;
                for (int j = 0; j < numGenes; j++) {
                        onesample.push_back(enteredData[j][i]);
                }
                MagicBall.add_sample(onesample, 1);
        }
        //have the classifier calculate stats
        MagicBall.done_entering();


        //reading in query data
        query_stream >> numGenes;
        query_stream >> numClassZero;
        query_stream >> numClassOne;
        double numQuerySamples = numClassZero + numClassOne;

        tempVal = 0.0;
        vector<vector<double>> enteredqData;

        //buffer all the data
        for (int i = 0; i < numGenes; i++) {
                vector<double> oneRow;
                for (int j = 0; j < numQuerySamples; j++) {
                        query_stream >> tempVal;
                        oneRow.push_back(tempVal);
                }
                enteredqData.push_back(oneRow);
        }
        //isolate each query data, and then classify it
        for (int i = 0; i < numQuerySamples; i++) {
                vector<double> onesample;
                for (int j = 0; j < numGenes; j++) {
                        onesample.push_back(enteredqData[j][i]);
                }
                cout << MagicBall.classify_sample(onesample) << "\t";
        }
        cout << endl;
}
*/
