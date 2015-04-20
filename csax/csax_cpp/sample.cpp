// Brett Fischler
// April 2015
// Sample Class Implementation

#include "sample.h"
#include <iostream>
#include <vector>
using namespace std;

Sample::Sample(int n) {
    genecount = n;

    vector<double> newGeneset;
    for (int i = 0; i < genecount; i++) {
        newGeneset.push_back(0);
    }
    geneset = newGeneset;
    c = -1;
}

Sample::Sample(int n, string newName, vector<double> newGenes) {
    genecount = n;

    vector<double> newGeneset;
    for (int i = 0; i < genecount; i++) {
        newGeneset.push_back(0);
    }
    geneset = newGeneset;
    setGenes(newGenes);
    c = -1;
    name = newName;
}

int Sample::getClass() {
    return c;
}

void Sample::setClass(int newC) {
    c = newC;
}
string Sample::getName() {
    return name;
}

void Sample::setGene(int index, double gene) {
    geneset[index] = gene;
}

void Sample::setGenes(vector<double> newGenes) {
    if ((signed)newGenes.size() != genecount) {
        cerr << "Sample class not given vector of correct length!" << endl;
    }
    for (int i = 0; i < genecount; i++) {
        geneset[i] = newGenes[i];
    }
}

double Sample::getGene(unsigned index) {
    return geneset[index];
}

void Sample::print() {
    for (auto i = geneset.begin(); i != geneset.end(); i++) {
        cout << *i << endl;
    }
}

