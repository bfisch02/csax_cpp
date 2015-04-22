#! ../bin/bash

valgrind --leak-check=full ../bin/FRaC -X res/training.arff -V res/training.arff -Q res/training.arff -M 1
