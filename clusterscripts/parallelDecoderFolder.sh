
#!/bin/bash
#PBS -l walltime=01:00:00 
#PBS -q short_cpuQ 
#PBS -v PROCESSES,THREADS,INPUT,OUTPUTFOLDER
#PBS -M michele.yin@studenti.unitn.it

MAIN=/home/michele.yin/HuffmanCODEC/bin/main.out
MODE=d
OUTPUT=${INPUT}.huf${THREADS}

TOTAL=$(($THREADS*$PROCESSES))
echo main executable ${MAIN}
echo mode ${MODE}
echo input folder ${INPUT}
echo output folder ${OUTPUT}
echo number of threads ${THREADS}
echo number of processes ${PROCESSES}
echo number of cores ${TOTAL}
echo results folder ${OUTPUTFOLDER}
module load mpich-3.2
module load openmpi-4.0.4
export OMP_PLACES=threads
# export OMP_PROC_BIND=spread
# export I_MPI_PIN_DOMAIN=scatter
# export I_MPI_DEBUG=5
# export I_MPI_PIN_ORDER=scatter
# export KMP_AFFINITY=verbose,scatter
time mpiexec --report-bindings -np ${PROCESSES} --map-by node:pe=${THREADS} --bind-to core ${MAIN} -${MODE} -r ${INPUT} ${OUTPUT}  #1> ${OUTPUTFOLDER}/deconding_result_${THREADS}_threads  2> ${OUTPUTFOLDER}/deconding_result_${THREADS}_threads_err


# How to run
# write INPUT=path to folder to encode
# write OUTPUTFOLDER=path to specify where to save stdout and stderr
# then copy and paste this command to submit the job
# for PROCESSES in 1 2 4; do for THREADS in 1 2 4 6 8 10 12 16 20 24; do export PROCESSES; export THREADS; export INPUT;export OUTPUTFOLDER; qsub -N Decoder${THREADS} -l select=1:ncpus=${THREADS}:mem=4gb -o ${OUTPUTFOLDER}/deconding_result_${THREADS}_threads -e ${OUTPUTFOLDER}/deconding_result_${THREADS}_threads_err '/home/michele.yin/HuffmanCODEC/clusterscripts/parallelDecoderFolder.sh'; done; done