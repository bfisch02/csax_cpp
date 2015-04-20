// Brett Fischler and Jacob Gerace
// April 2015
// Sample Class Interface

#ifndef SAMPLE_H
#define SAMPLE_H

#include <iostream>
using namespace std;
#include<vector>

class Sample {
    public:
        Sample(int genecount);
        Sample(int genecount, string newName, vector<double> newGenes);
        int getClass();
        void setClass(int c);
        void setGene(int index, double gene);
        string getName();
        void setGenes(vector<double> newGenes);
        void print();
        double getGene(unsigned index);
    private:
        vector<double> geneset;
        string name;
        int genecount;
        int c;
};

#endif
