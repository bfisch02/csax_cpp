// Brett Fischler and Jacob Gerace
// April 2015
// Gene Set Manager Implementation

#include "genesetmanager.h"
#include <map>
#include <iostream>
#include <math.h>
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
    sortByMedian();
    double total_score = 0;
    double cur_score = 0;
    for (unsigned i = 0; i < genesets.size(); i++) {
        if (ES->count(genesets[i]->name) == 0) {
            continue;
        } else {
            cur_score = ES->at(genesets[i]->name);
            total_score += cur_score * pow(gamma, i);
        }
    }
    return total_score;
}
