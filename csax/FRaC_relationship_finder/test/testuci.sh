#! /bin/bash

#trap 'killall evaluate' SIGINT

CVFOLDS=100
SHUFFLES=0
ENSEMBLE=0$1

OPT1=$2
OPT2=$3
OPT3=$4
OPT4=$5

testSet () {
  SETNAME=$1
  
  ../bin/FRaC -E res/uci/$SETNAME -N $CVFOLDS -S $SHUFFLES -M $ENSEMBLE $OPT1 $OPT2 $OPT3 $OPT4

  if [ "$?" -ne "0" ]
    then
      echo "Failure: $SETNAME"
    else
      AUROC=$(./evaluate/auroc res/uci/$SETNAME/frac_out)
      echo "$SETNAME: $AUROC"
    fi
}

echo "Ensemble: $ENSEMBLE"
echo ""

if [ "$OPT1" = "all" ]
  then
    allSets=res/uci/*

    for i in $allSets
      do
        testSet ${i##*/} &
      done
  else

#testSet "abalone"
testSet "acute" &
#testSet "audiology"
testSet "balance-scale" &
testSet "car" &
testSet "cmc" &
#testSet "dermatology" &
testSet "ecoli" &
testSet "glass" &
testSet "haberman" &
#testSet "horse-colic" &
testSet "iris" &
#testSet "letter-recognition" &
#testSet "magic" &
#testSet "mammographic-masses" &
testSet "nursery" &
#testSet "page-blocks" &
#testSet "parkinsons" &
#testSet "pima-indians-diabetes" &
#testSet "poker" &
#testSet "secom" &
#testSet "spambase" &

testSet "tae" &

testSet "tic-tac-toe" &
testSet "wine" &
testSet "yeast" &
testSet "zoo" &

  fi

wait

