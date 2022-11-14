
#!/bin/bash

#PBS -l select=1:ncpus=4:ompthreads=1:mpiprocs=4:mem=4gb
#PBS -l walltime=00:00:30
#PBS -q short_cpuQ
module load mpich-3.2 
module load openmpi-4.0.4
# export OMP_PLACES=threads
# # export OMP_PROC_BIND=spread
# # export I_MPI_PIN_DOMAIN=scatter
# # export I_MPI_DEBUG=5
# export I_MPI_PIN_ORDER=scatter
# export KMP_AFFINITY=verbose,scatter
mpiexec --report-bindings -np 4 --map-by node:pe=1 --bind-to core /home/michele.yin/HuffmanCODEC/bin/folder.out /home/michele.yin/HuffmanCODEC/src/parallel /home/michele.yin/HuffmanCODEC/src/parallel/shit/