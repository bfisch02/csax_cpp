// Brett Fischler and Jacob Gerace
// April 2015
// Gene Set Manager Implementation

#include "genesetmanager.h"
#include <map>
#include <iostream>
#include <math.h>
#include <algorithm>
using namespace std;

/*Sort function using C++ included sort*/
bool sortfunction (GeneSet *a, GeneSet *b) {
    unsigned size;
    if (a->median == -1) {
        std::sort(a->rankings.begin(), a->rankings.end());
        size = a->rankings.size();
        if (size % 2 == 0) {
            a->median = (a->rankings[size / 2 - 1] +
                a->rankings[size / 2]) / 2;
        } else {
            a->median = a->rankings[size / 2];
        }
    }
    if (b->median == -1) {
        std::sort(b->rankings.begin(), b->rankings.end());
        size = b->rankings.size();
        if (size % 2 == 0) {
            b->median = (b->rankings[size / 2 - 1] +
                b->rankings[size / 2]) / 2;
        } else {
            b->median = b->rankings[size / 2];
        }
    }
    return a->median < b->median;
}

GeneSetManager::GeneSetManager() {
}

/*Adds a ranking to the manager */
void GeneSetManager::addRankingToGeneset(int ranking, string geneset)
{
    GeneSet *g;
    if (genesetmap.count(geneset) == 0) {
        g = new GeneSet;
        g->name = geneset;
        g->median = -1; // Flag that median has not been found
        genesets.push_back(g);
        genesetmap[geneset] = g;
    }
    g = genesetmap[geneset];
    g->rankings.push_back(ranking);
}

/*Sorting wrapper function */
void GeneSetManager::sortByMedian()
{
    std::sort(genesets.begin(), genesets.end(), sortfunction);
}

/*Calculates anomaly score for a given enrichment score map using a gamma value
 * */
double GeneSetManager::getAnomalyScore(double gamma, map<string, double> *ES)
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
