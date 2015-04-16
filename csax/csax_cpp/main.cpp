// Brett Fischler
// April 2015
// Sample Class Implementation

#include "sample.h"
#include <stdlib.h>
#include <fstream>
#include <iostream>
using namespace std;

Sample *getData(char *matrix_file, string metadata_file);

int main(int argc, char **argv) {
    if (argc < 3) {
        cerr << "Usage: ./a.out matrix metadata" << endl;
        exit(1);
    }
    Sample *data = getData(argv[1], argv[2]);
}

Sample *getData(char *matrix_file, string metadata_file)
{
    ifstream metadata, matrix;
    metadata.open(metadata_file);
    matrix.open(matrix_file);
    string line;
    if (matrix.is_open()) {
        while (getline(matrix, line, ')) {
            cout << line << endl;
        }
    }
    metadata.close();
    return NULL;
}
