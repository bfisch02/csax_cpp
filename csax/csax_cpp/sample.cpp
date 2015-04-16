// Brett Fischler
// April 2015
// Sample Class Implementation

#include "sample.h"
#include <iostream>
using namespace std;

Sample::Sample(int n) {
    genecount = n;
    geneset = new float[genecount];
    for (int i = 0; i < genecount; i++) {
        geneset[i] = 0;
    }
}

int Sample::getClass() {
    return c;
}

void Sample::setClass(int c) {
    this->c = c;
}

void Sample::setGene(int index, float gene) {
    geneset[index] = gene;
}
