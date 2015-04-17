//k nearest neighbors class implementation
//Jacob Gerace
using namespace std;
/*TODO: Not all these inclusions necessary*/
#include <stdio.h>
#include <iostream>
#include <unordered_map>
#include <list>
#include <vector>
#include <set>
#ifndef classifier_H
#define classifier_H
//comments for each function found in the .cpp implementation
class CSAX
{
        public:
                CSAX(int new_kNearest, int new_numGenes, int new_numGenesUse);
                /*
                void add_sample(vector<double> values, int classtype);
                int classify_sample(vector<double> gene_vals);
                void done_entering();
                */

        private:
                /*
                double calculate_t_value(vector<double> class_zero_vals,
                                                   vector<double> class_one_vals);
                double calculate_distance(vector<double> new_sample,
                                                      vector<double> train_sample);
                void print();

                int kNearest;
                int numGenes;
                int numGenesUse;
                int numZeroSamples;
                int numOneSamples;
                //training data
                vector<vector<double>> class_zero_data;
                vector<vector<double>> class_one_data;
                //training data stats
                vector<double> tvals;
                vector<bool> best_k;
                */
};
#endif
