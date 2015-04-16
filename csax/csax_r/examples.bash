#!/bin/bash

# This script shows some examples of microarray anomaly detection.  The task
# was created from a subset of genes in one of the data sets in our compendium.


cd `dirname $0`  
echo "Working directory is: `pwd`"

echo "Run 'make':"
make # make sure all programs compile

mkdir -p examples.output # write output here

# Run LOF:
echo "Run LOF..."
lof.r -L 2 examples.input/example.training.set examples.input/example.test.set examples.output/lof.g

# Print resulting AUC:
echo -ne "\nRESULT: LOF(l2) AUC=`auc.r examples.input/example.test.set.labels examples.output/lof.g`\n\n"

# Run SVM
echo "Run One-class SVM:"
svm.r examples.input/example.training.set examples.input/example.test.set examples.output/svm.g
echo -ne "\nRESULT: SVM AUC=`auc.r examples.input/example.test.set.labels examples.output/svm.g`\n\n"

# FRaC
echo "Run FRaC:"
frac.r -O examples.output/frac examples.input/example.training.set examples.input/example.test.set examples.output/frac.g
echo -ne "\nRESULT: FRaC AUC=`auc.r examples.input/example.test.set.labels examples.output/frac.g`\n\n"

# CSAX
echo "Run CSAX:"
# (run bagging iterations 1 through 5 only)
csax.r -B 5 examples.input/example.training.set examples.input/example.test.set examples.input/reactome.gmt examples.output/csax examples.output/csax.g
echo -ne "\nRESULT: CSAX AUC=`auc.r examples.input/example.test.set.labels examples.output/csax.g`\n\n"

echo "Compile CSAX gene set rankings:"
# (run bagging iterations 1 through 5 only)
csax.genesets.r -B 5  examples.input/reactome.gmt examples.output/csax examples.output/csax.genesets.xls

echo -ne "ALL DONE.  Wrote to examples.output\n"


