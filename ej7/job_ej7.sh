#!/bin/bash
echo "NODELIST $SLURM_NODELIST"
prun ./ej5mejorado $1
