#! /bin/bash

#trap 'killall evaluate' SIGINT

CVFOLDS=2
SHUFFLES=0
ENSEMBLE=0$3

CORRECTION=0

DIR=$1
TOP=$2

nsStats() {
  awk '{ if ($1 == 0) {sum0 += $2; n0++} else {sum1 += $2; n1++} } END { print sum0 / n0 "\t" sum1 / n1 "\t" (sum0 + sum1) / (n0 + n1); }' < $1
}

auroc() {
  ../evaluate/auroc $1
}

allStats () {
  FILENAME=$1
  
  AUROC=$(auroc $FILENAME)
  NSSTAT=$(nsStats $FILENAME)
  echo "$FILENAME\t$AUROC\t$NSSTAT"
}

#Takes the directory as input
runTest () {
  SETDIR=$1

  #Run uncorrected
  ../../bin/FRaC -E $SETDIR -N $CVFOLDS -S $SHUFFLES -M $ENSEMBLE
  
  if [ "$?" -ne "0" ]
    then
      echo "Failure: $SETNAME"
    else
      cp $SETDIR/frac_out $SETDIR/frac_out_nc
    fi

  #Run corrected
  ../../bin/FRaC -E $SETDIR -N $CVFOLDS -S $SHUFFLES -M $ENSEMBLE -U

  if [ "$?" -ne "0" ]
    then
      echo "Failure: $SETNAME"
    else
      cp $SETDIR/frac_out $SETDIR/frac_out_c
    fi

}

echo "SET\tCorrected\tAUROC\tMean NS normal\tMean NS abnormal\tMean NS"

#Run FRaC
for i in $(seq 0 $TOP)
  do
    runTest "$DIR/$i" &
  done

wait

#Run analysis

for i in $(seq 0 $TOP)
  do
    for j in $(seq $i $TOP)
      do
        #Remove any existing accumulation files.
        rm -f "$DIR/$i-$j-nc" "$DIR/$i-$j-c"
        for k in $(seq $i $j)
          do
#echo "copying $k to $i-$j"
            cat "$DIR/$k/frac_out_nc" >> "$DIR/$i-$j-nc"
            cat "$DIR/$k/frac_out_c" >> "$DIR/$i-$j-c"
          done
        allStats "$DIR/$i-$j-nc" "No"
        allStats "$DIR/$i-$j-c" "Yes"
      done
  done

