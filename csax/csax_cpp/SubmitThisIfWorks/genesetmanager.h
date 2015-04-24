// Brett Fischler and Jacob Gerace
// April 2015
// Gene Set Manager Interface

#ifndef GENESETMANAGER_H
#define GENESETMANAGER_H

#include <map>
#include <vector>
#include <iostream>
using namespace std;

struct GeneSet {
    string name;
    vector<int> rankings;
    float median;
};

class GeneSetManager {
    public:
        GeneSetManager();
        void addRankingToGeneset(int ranking, string geneset);
        void sortByMedian();
        double getAnomalyScore(double gamma, map<string, double> *ES);
    private:
        vector<GeneSet*> genesets;
        map<string, GeneSet*> genesetmap;
};

#endif
