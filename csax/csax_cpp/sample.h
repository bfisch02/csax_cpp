// Brett Fischler
// April 2015
// Sample Class Interface

#ifndef SAMPLE_H
#define SAMPLE_H

#include<vector>

class Sample {
    public:
        Sample(int genecount);
        int getClass();
        void setClass(int c);
        void setGene(int index, float gene);
    private:
        vector<float> geneset;
        int genecount;
        int c;
};

#endif
