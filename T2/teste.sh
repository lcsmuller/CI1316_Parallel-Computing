#!/bin/bash

NNODOS=$1

mpirun -np $NNODOS ./myBroadcast 1 4000 $NNODOS -r $2
