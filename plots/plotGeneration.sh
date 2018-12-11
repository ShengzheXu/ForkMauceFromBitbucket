#!/bin/bash

experiments_folder="./experiments"
results_folder="$experiments_folder/results"
plot_folder="$experiments_folder/plots"

mkdir -p "$plot_folder"

if [ $# -eq 0 ]; then
    extension="png"
else
    extension=$1
fi

./plotResults.py --output="$plot_folder/nodes_all.$extension"  "$results_folder" nodes --nodes=11 --timesteps=10000 scql mauce rnd llr
./plotResults.py --output="$plot_folder/nodes_xs.$extension"   "$results_folder" nodes --nodes=11 --timesteps=10000 scql mauce

./plotResults.py --output="$plot_folder/mines1.$extension"     "$results_folder" mines --seed=1 scql mauce rnd llr
./plotResults.py --output="$plot_folder/mines2.$extension"     "$results_folder" mines --seed=2 scql mauce rnd llr
./plotResults.py --output="$plot_folder/mines3.$extension"     "$results_folder" mines --seed=1000 scql mauce rnd llr
./plotResults.py --output="$plot_folder/mines4.$extension"     "$results_folder" mines --seed=10000 scql mauce rnd llr
./plotResults.py --output="$plot_folder/mines.$extension"      "$results_folder" mines --seed=avg scql mauce rnd llr

./plotResults.py --output="$plot_folder/wind.$extension"       "$results_folder" wind scql mauce rnd llr
