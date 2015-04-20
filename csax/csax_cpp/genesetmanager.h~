// Brett Fischler and Jacob Gerace
// April 2015
// Gene Set Manager Interface

#ifndef GENESETMANAGER_H
#define GENESETMANAGER_H

struct GeneSet {
    string name;
    vector<int> rankings;
};

class GeneSetManager {
    public:
        GeneSetManager();
        void addRankingToGeneset(int ranking, string geneset);
        void sortByMedian();
    private:
        vector<GeneSet*> genesets;
        map<string, GeneSet*> genesetmap;
};

#endif
