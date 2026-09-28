#!/bin/bash
mkdir -p resultados
for rep in 1 2 3; do
  for N in 100000 1000000 10000000 100000000; do
    for p in 1 2 4 8 16 32; do
      sbatch -J ej7-09 --dependency=singleton -n $p -t 00:10:00 \
             -o resultados/N${N}_p${p}_r${rep}.out job_ej7.sh $N
    done
  done
done
