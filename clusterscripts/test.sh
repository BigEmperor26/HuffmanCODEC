
#!/bin/bash

#PBS -l select=1:ncpus=12:ompthreads=3:mpiprocs=4:mem=4gb
#PBS -l walltime=0:00:10
#PBS -q short_cpuQ
module load mpich-3.2 
module load openmpi-4.0.4
export OMP_PLACES=threads
# export OMP_PROC_BIND=spread
# export I_MPI_PIN_DOMAIN=scatter
# export I_MPI_DEBUG=5
# export I_MPI_PIN_ORDER=scatter
# export KMP_AFFINITY=verbose,scatter
mpiexec --report-bindings -np 4 --map-by socket:pe=3 --bind-to core /home/michele.yin/HuffmanCODEC/a.out