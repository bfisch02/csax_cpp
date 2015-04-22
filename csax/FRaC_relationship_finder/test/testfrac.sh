#! ../bin/bash

#Test FRaC with validation set
valgrind ../bin/FRaC -X res/training.arff -V res/validation.arff -Q res/query.arff
