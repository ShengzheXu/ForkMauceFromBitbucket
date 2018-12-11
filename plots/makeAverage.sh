#!/bin/bash

for i in rnd scl llr mauce; do
    ./transform.py $(ls experiments/results/mines/$i/experiments* | grep -v avg) "./experiments/results/mines/$i/experiments=100_seed=avg_timesteps=40000"
done
