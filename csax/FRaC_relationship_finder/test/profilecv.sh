#! /bin/bash

#Test FRaC with cross validation
valgrind --tool=callgrind ../bin/FRaC -X res/trainingandvalidation.arff -Q res/query.arff -N 9
