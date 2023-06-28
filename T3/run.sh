#!/bin/bash

for i in {1..4}; do
    mkdir "${i}threads"
    cp mmul ./"${i}threads"/mmul
    cp myMmul-slurm.sh ./"${i}threads"/myMmul-slurm.sh
    cd "${i}threads"
    for j in {1..10}; do
        sbatch --exclusive -N "${i}" myMmul-slurm.sh
    done
    cd ..
done
