#! /bin/bash

#Test FRaC with cross validation
valgrind ../bin/FRaC -X res/trainingandvalidation.arff -Q res/query.arff
