// Brett Fischler and Jacob Gerace
// April 2015
// Sample Class Interface

#ifndef SAMPLE_H
#define SAMPLE_H

using namespace std;
#include<vector>

class Sample {
    public:
        Sample(int genecount);
        Sample(int genecount, vector<float> newGenes);
        int getClass();
        void setClass(int c);
        void setGene(int index, float gene);
        void setGenes(vector<float> newGenes);
        void print();
    private:
        vector<float> geneset;
        int genecount;
        int c;
};

#endif
