// Brett Fischler and Jacob Gerace
// April 2015
// Gene Set Manager Implementation

#include "genesetmanager.h"
#include <map>
#include <iostream>
using namespace std;

GeneSetManager::GeneSetManager() {

}

void GeneSetManager::addRankingToGeneset(int ranking, string geneset)
{
    GeneSet *g;
    if (genesetmap.count(geneset) == 0) {
        g = new GeneSet;
        g->name = geneset;
        genesets.push_back(g);
        genesetmap[geneset] = g;
    }
    g = genesetmap[geneset];
    g->rankings.push_back(ranking);
}

void GeneSetManager::sortByMedian()
{
    // Sort genesets vector by median
}

double GeneSetManager::getAnomalyScore(float gamma, map<string, float> *ES)
{

}
