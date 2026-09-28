#!/bin/bash
mkdir -p lat
for rep in $(seq 1 10); do
  sbatch -J ej6-09 --dependency=singleton -N 1 -n 2 -w compute-small-01-01 \
         -o lat/mismonodo_r${rep}.out job_lat.sh
  sbatch -J ej6-09 --dependency=singleton -N 2 -n 2 -w compute-small-01-01,compute-small-01-02 \
         -o lat/mismorack_r${rep}.out job_lat.sh
  sbatch -J ej6-09 --dependency=singleton -N 2 -n 2 -w compute-small-01-01,compute-small-02-01 \
         -o lat/distintorack_r${rep}.out job_lat.sh
done
