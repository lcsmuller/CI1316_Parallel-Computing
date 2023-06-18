#!/bin/bash

NNODOS=$1

mpirun -np $NNODOS broadcast 1 4000 $NNODOS -r $2
