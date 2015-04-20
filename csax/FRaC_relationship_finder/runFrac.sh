#! /bin/bash

set -e
trap "" HUP

#These variables need to be set for each dataset
datasetname="schizophrenia"
indirectory="../snp/schizophrenia_cds/reduced/"
featurePrefix="SNP_A-"
normalClassName="norm"

#Change this to "local" to use a native architecture build (will cause incompatibilies between different processors)
fracmode=release

#FRaC parameters
#% of features to heuristically filter (must give 2 digits).
kl=0
#Cross validation folds (Higher values increase accuracy. but greatly increase runtime).
n=8
#Minimum bucket size to consider significant.  A parameter used to terminate decision tree construction, you probably don't need to adjust this.
b=8

#Derived values
outsuffix=N_"$n"_kl_"$kl"_B_"$b"
outlogfile=$datasetname"_log_"$outsuffix
outgraph=$datasetname"_graph_"$outsuffix
program=./bin$fracmode/FRaC

#Build

#Untar and compile FRaC if it doesn't exist
if [ ! -e $program ]
  then
    #Extract FRaC
    tar -xzvf FRaC.tar.gz
    #Compile FRaC
    echo "Making FRaC..."
    make frac mode=$fracmode -j
    echo "Build OK."
  fi

echo "Running FRaC as a background job.  See \"$outlogfile\" to monitor progress."

#Code to run in the background.
evaluate () {
  { time $program -E $indirectory -N $n -l -K 0.$kl -B $b -L $indirectory"/frac_out_termwise" ; } &> $outlogfile
  echo "AUROC:" &>> $outlogfile
  test/evaluate/auroc $indirectory/frac_out &>> $outlogfile
  echo "Running relationship finder:" &>> $outlogfile
  python3 TreeOutputParser.py --median -F --prefix=$featurePrefix --normal=$normalClassName $indirectory 2>> $outlogfile 1> $outgraph
  echo "Relationship finder finished.  Please see \"$outgraph\" for results." &>> $outlogfile
}

#Set stack limit to unlimited
ulimit -s unlimited
evaluate &

