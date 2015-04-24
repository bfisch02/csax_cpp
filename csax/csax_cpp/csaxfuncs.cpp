// Brett Fischler and Jacob Gerace
// April 2015
// Helper function implementations for CSAX algorithm

#include "csaxfuncs.h"
//#include <boost/algorithm/string/split.hpp>
#include <algorithm>
#include "genesetmanager.h"
#include "sample.h"
#include <vector>
#include <map>
#include <fstream>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <cstring>
#include <sys/types.h>
#include <dirent.h>
using namespace std;

struct GeneScoreList {
    string gene;
    vector<double> scores;
};

string OUTPUT_DIR = "";

// Declarations
void initializeOutputDir(string output_dir);
vector<GeneScoreList *> runFRaC(SampleList traindata, string testfile,
        bool use_all);
vector<map<string, double>*> runGSEA(string genesets_file,
        vector<GeneScoreList *> genescores);
void CSAX_iteration(SampleList traindata, SampleList testdata, string testfile,
        string genesets_file, vector<GeneSetManager *> managers);
void samplesToFile(SampleList samples, string filename, double percent_to_add);
vector<GeneScoreList *> parseFRaCOutput(string fracFileP);
map<string, double> *parseGSEAOutput(string gseaFileP);
void writeGSEAInput(vector<GeneScoreList *> genescores, unsigned index,
        string filename);

/* Function runCSAX:
 * Takes in parsed data from the training data and testdata
 * respectively
 * Takes in The filepointer for the testfile, and the parsed data (currently
 * redundant)
 * Also the output directory, and the number of bagging iterations
 * Also takes in gamma (0.95 typically used)
 *
 * Runs CSAX on the full training data, and then performs one iteration on a
 * random half of the data, num_bags number of iterations
 */
void runCSAX(SampleList traindata, SampleList testdata, string testfile,
        string genesets_file, string output_dir, int num_bags, double gamma)
{
    cout << "Running CSAX on whole training set" << endl;
    initializeOutputDir(output_dir);
    // First, we have to process the full training data
    vector<GeneScoreList *> genescores =
        runFRaC(traindata, testfile, true);

    // Run GSEA as well
    vector<map<string, double>*> ES = runGSEA(genesets_file, genescores);

    vector<GeneSetManager*> managers;
    for (unsigned i = 0; i < testdata.data.size(); i++) {
        managers.push_back(new GeneSetManager());
    }

    //int num_bags = 5;
    for (int b = 0; b < num_bags; b++) {
        cout << "Running csax on iteration: " << b << endl;
        CSAX_iteration(traindata, testdata, testfile, genesets_file, managers);
    }

    string output_file = OUTPUT_DIR + "csax_anomaly_scores";
    ofstream f;
    f.open(output_file);

    double cur_score = 0;
    for (unsigned i = 0;i  < testdata.data.size(); i++) {
        cur_score = managers[i]->getAnomalyScore(gamma, ES[i]);
        f << testdata.data[i]->getName() << "\t" << cur_score << endl;
    }

    f.close();
    cout << "CSAX Finished! output in " << output_file << endl;

}

/* Sets up the output directory */
void initializeOutputDir(string output_dir)
{
    OUTPUT_DIR = output_dir;
    if (OUTPUT_DIR[OUTPUT_DIR.length() - 1] != '/') {
        OUTPUT_DIR += '/';
    }
}

/* CSAX_iteration is passed all the data from runCSAX
 * Calls frac on the given train and test data. Gets results from frac, and
 * then calls GSEA on that frac output
 */
void CSAX_iteration(SampleList traindata, SampleList testdata, string testfile,
        string genesets_file, vector<GeneSetManager *> managers)
{

    // Run FRaC on sample of traindata
    vector<GeneScoreList *> genescores =
        runFRaC(traindata, testfile, false);

    // Run GSEA on output
    vector<map<string, double> *> enrichmentscores =
        runGSEA(genesets_file, genescores);

    int rank;
    // For each test sample
    for (unsigned i = 0; i < testdata.data.size(); i++) {
        rank = 1; // TODO: Check if first rank should be 1 or 0
        for (std::map<string, double>::iterator it=enrichmentscores[i]->begin();
                it != enrichmentscores[i]->end(); it++) {
            managers[i]->addRankingToGeneset(rank++, it->first);
        }
    }
}

/* Takes in the training data and a file pointer to the test data
 * Creates a traindata file, passes in that file and the testfile to Frac
 * Parses the outputfile from Frac and returns it as a list of GeneScoreLists
 */
vector<GeneScoreList *> runFRaC(SampleList traindata, string testfile,
        bool use_all)
{
    (void)testfile;
    string trainfile = "trainfile";
    if (use_all) {
        samplesToFile(traindata, trainfile, 1);
    } else {
        samplesToFile(traindata, trainfile, .5);
    }

    cout << "Calling FRaC" << endl;
    string command = "./frac.r " + trainfile + " " + testfile + " FRaC_output";
    system(command.c_str());
    string fracFileP = "output_location/ns.gz";
    command = "gzip -d " + fracFileP;
    system(command.c_str());


    string decompfracFileP = OUTPUT_DIR + "ns";

    vector<GeneScoreList *> genescores = parseFRaCOutput(decompfracFileP);

    //need to remove FRaC's files otherwise it will reuse them the next
    //iteration
    remove(fracFileP.c_str());
    remove(decompfracFileP.c_str());

    return genescores;
}

/* Parses the GSEA output, returns it in a map representation
 */
map<string, double> *parseGSEAOutput(string gseaFileP)
{
    map<string, double> *inputBuffer;
    ifstream matrix;
    matrix.open(gseaFileP);
    string line = "";
    string genesetname;
    double score;
    string junk;
    if (matrix.is_open()) {
        //Don't need the header, just call getline to remove it
        getline(matrix, line);
        inputBuffer = new map<string, double>;
        while (matrix >> genesetname) {
            matrix >> junk >> junk >> score >> junk >> junk >> junk >> junk >> junk >> junk >> junk >> junk;
            (*inputBuffer)[genesetname] = score;
        }
    }
    return inputBuffer;
}

/* Parses FRaC output from a given fileponter
 */
vector<GeneScoreList *> parseFRaCOutput(string fracFileP)
{
    vector<GeneScoreList *> inputBuffer;
    ifstream matrix;
    matrix.open(fracFileP);
    string line = "";
    string name;

    vector<string> nameGenes;
    double tempdouble = 0.0;
    unsigned numNames = 0;
    GeneScoreList * p;
    if (matrix.is_open()) {
        //Don't need the header, just call getline to remove it
        getline(matrix, line);
        istringstream lineStream (line);
        while (lineStream >> name) {
            numNames++;
         }
        while (matrix >> name) {
            vector<double> oneRow;
            for (unsigned i = 0; i < numNames; i++) {
                matrix >> tempdouble;
                oneRow.push_back(tempdouble);
            }
            p = new GeneScoreList;
            p->gene = name;
            p->scores = oneRow;
            inputBuffer.push_back(p);
        }
    }
    return inputBuffer;
}

/* Given the output from frac (genescores) and the genesets_file (like
 * reactome), returns the output from GSEA in an internal representation
 */
vector<map<string, double>*> runGSEA(string genesets_file,
        vector<GeneScoreList *> genescores)
{
    (void)genesets_file;
    (void)genescores;
    string inputfile;
    vector<map<string, double>*> enrichment_scores;
    string outputfolder;
    string gseajar = "gsea/gsea.jar";
    unsigned num_tests = genescores[0]->scores.size();
    for (unsigned i = 0; i < num_tests; i++) {
        inputfile = "gseainput" + std::to_string(i + 1) + ".rnk";
        writeGSEAInput(genescores, i, inputfile);

        outputfolder = "output_location/gsea_output";


        /*GSEA has an ugly input command */
        string cmd = "java -cp " + gseajar + " -Xmx1g xtools.gsea.GseaPreranked -gmx " + genesets_file + " -rnk " + inputfile + " -out " + outputfolder + " -rnd_seed 9141976 -rpt_label csax -collapse false -mode Max_probe -norm meandiv -nperm 1000 -scoring_scheme weighted -include_only_symbols true -make_sets false -plot_top_x 0 -set_max 500 -set_min 7 -zip_report false -gui false 1>/dev/null ";

        cout << "calling GSEA" << endl;
        system(cmd.c_str());

        DIR *directory = opendir(outputfolder.c_str());
        struct dirent *entry;
        if (directory == NULL) {
            cerr << "No gsea output... Abort!" << endl;
            exit(1);
        }
        while ((entry = readdir(directory)) != NULL) {
            if (entry->d_name[0] == 'c') {
                break;
            }
        }
        string suffix = string(entry->d_name).substr(19);
        string gseaoutput = outputfolder + "/" + entry->d_name +
            "/gsea_report_for_na_pos_" + suffix + ".xls";

        enrichment_scores.push_back(parseGSEAOutput(gseaoutput));
        cmd = "rm -r " + outputfolder;
        system(cmd.c_str());
    }

    return enrichment_scores;
}

/* Writes a sample list to file
 * Used to create an input file for a FRaC call
 * Randomly chooses percent_to_add of the queries to write to file
 */
void samplesToFile(SampleList samples, string filename, double percent_to_add)
{
    ofstream f;
    double fractionBag = percent_to_add;
    unsigned numTraining = samples.data.size();
    unsigned numTrue = fractionBag * numTraining;
    vector<bool> truthVector;

    f.open(filename);

    //make a random permutation parallel array
    for (unsigned i = 0; i < numTrue; i++) {
        truthVector.push_back(true);
    }
    for (unsigned i = numTrue; i < numTraining; i++) {
        truthVector.push_back(false);
    }
    std::random_shuffle (truthVector.begin(), truthVector.end());

    //output training data to a file
    for (unsigned i = 0; i < samples.data.size(); i++) {
        if (truthVector[i]) {
            f << samples.data[i]->getName() << "\t";
        }
    }
    f << endl;

    unsigned numGenes = samples.geneNames.size();
    for (unsigned j = 0; j < numGenes; j++) {
        f << samples.geneNames[j] << "\t";
        for (unsigned i = 0; i < samples.data.size(); i++) {
            if (truthVector[i])
                f << samples.data[i]->getGene(j) << "\t";
        }
        f << endl;
    }

    f.close();
}

/* Given one index, writes one genescore to a file to be used with GSEA
 */
void writeGSEAInput(vector<GeneScoreList *> genescores, unsigned index,
        string filename)
{
    ofstream f;
    f.open(filename);
    for (unsigned i = 0; i < genescores.size(); i++) {
        f << genescores[i]->gene << '\t'
          << genescores[i]->scores[index] << endl;
    }
    f.close();
}
