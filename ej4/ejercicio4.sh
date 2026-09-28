#!/bin/bash

#SBATCH -J ejemplo-sbatch-09              # Job name
#SBATCH -o ejemplo-sbatch-09-%j.stdout         # Name of stdout output file (%j expands to jobId)
#SBATCH -N 2                 # Total number of nodes requested
#SBATCH -n 4                 # Total number of mpi tasks requested
#SBATCH -t 01:00:00           # Run time (hh:mm:ss) - 1 hours

# Launch MPI-based executable
