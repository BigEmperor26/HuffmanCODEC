
#!/bin/bash

#PBS -l walltime=00:05:00 
#PBS -q short_cpuQ 
#PBS -v THREADS,INPUT,OUTPUTFOLDER
#PBS -M michele.yin@studenti.unitn.it

OUTPUTFOLDER=${OUTPUTFOLDER%/}

MAIN=/home/michele.yin/HuffmanCODEC/bin/main.out
MODE=e
OUTPUT=${INPUT}.huf${THREADS}

echo main executable ${MAIN}
echo mode ${MODE}
echo input folder ${INPUT}
echo output folder ${OUTPUT}
echo number of threads ${THREADS}
echo results folder ${OUTPUTFOLDER}

module load mpich-3.2
module load openmpi-4.0.4
export OMP_PLACES=threads
# export OMP_PROC_BIND=spread
# export I_MPI_PIN_DOMAIN=scatter
# export I_MPI_DEBUG=5
# export I_MPI_PIN_ORDER=scatter
# export KMP_AFFINITY=verbose,scatter
time mpiexec --report-bindings -np 1 -display-devel-map --map-by node:pe=${THREADS} --bind-to core ${MAIN} -${MODE} ${INPUT} ${OUTPUT} #1> ${OUTPUTFOLDER}/enconding_result_${THREADS}_threads  2> ${OUTPUTFOLDER}/enconding_result_${THREADS}_threads_err

# How to run
# write INPUT=path to file to encode
# write OUTPUTFOLDER=path to specify where to save stdout and stderr
# then copy and paste this command to submit the job
# for THREADS in 1 2 4 6 8 10 12 16 20 24 32 48 64 96; do export THREADS; export INPUT;export OUTPUTFOLDER; qsub  -N Encoder${THREADS} -l select=1:ncpus=96:ompthreads=${THREADS}:mpiprocs=1:mem=4gb -o ${OUTPUTFOLDER}/enconding_result_${THREADS}_threads -e ${OUTPUTFOLDER}/enconding_result_${THREADS}_threads_err '/home/michele.yin/HuffmanCODEC/clusterscripts/parallelEncoderFile.sh'; done