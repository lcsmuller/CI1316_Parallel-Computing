#!/bin/bash

<<<<<<< HEAD
NNODOS=$1

mpirun -np $NNODOS broadcast 1 4000 $NNODOS -r $2
=======
mpirun -np $1 broadcast 1 4000 -r
>>>>>>> c5851d27c021cbab19a5966be0c7675b8ea926c9
