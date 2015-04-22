#! /bin/bash

ENSEMBLE=$1

CVFOLDS=0
SHUFFLES=0
ENSEMBLE=$1

testSet () {
  SETNAME=$1
  
  ../bin/FRaC -E res/$SETNAME -N $CVFOLDS -S $SHUFFLES $ENSEMBLE

  if [ "$?" -ne "0" ]
    then
      echo "Failure: $SETNAME"
    else
      AUROC=$(./evaluate/auroc res/$SETNAME/frac_out)
      echo "$SETNAME: $AUROC"
    fi
}

testSet "trivial"
testSet "trivial2"
testSet "trivial3"
testSet "trivial4"
testSet "trivial5"

