
echo "Dataset\tReal\tNominal"
for f in res/uci/*
  do
    real=$(cat $f/metadata | grep real | wc -l)
    nominal=$(cat $f/metadata | grep nominal | wc -l)
    echo $f"\t"$real"\t"$nominal
  done
