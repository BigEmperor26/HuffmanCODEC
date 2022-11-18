
#!/bin/bash

#PBS -l walltime=00:20:00 
#PBS -q short_cpuQ 
#PBS -v THREADS,INPUT,OUTPUTFOLDER
#PBS -M michele.yin@studenti.unitn.it

OUTPUTFOLDER=${OUTPUTFOLDER%/}

MAIN=/home/michele.yin/HuffmanCODEC/bin/main.out
MODE=e
OUTPUT=${INPUT}.huf${THREADS}

echo $(date)
echo main executable ${MAIN}
echo mode ${MODE}
echo input file ${INPUT}
echo output file ${OUTPUT}
echo number of threads ${THREADS}
echo results folder ${OUTPUTFOLDER}

module load mpich-3.2
module load openmpi-4.0.4
export OMP_PLACES=threads
export OMP_NUM_THREADS=${THREADS}
time mpiexec --report-bindings -display-devel-map --map-by socket:pe=${THREADS} -np 1 ${MAIN} -${MODE} -l ${INPUT} ${OUTPUT} #1> ${OUTPUTFOLDER}/enconding_result_${THREADS}_threads  2> ${OUTPUTFOLDER}/enconding_result_${THREADS}_threads_err

# How to run
# write INPUT=path to file to encode
# write OUTPUTFOLDER=path to specify where to save stdout and stderr
# then copy and paste this command to submit the job
# OUTPUT will be in the same folder as INPUT
# OUTPUTFOLDER will be created if it does not exist and it will contain the results of the for each run
# nohup $(for RUN in $(seq 1 3); do for THREADS in 1 2 4 6 8 10 12 16 20 24; do mkdir -p ${OUTPUTFOLDER}/run_${RUN} ;export THREADS; export INPUT;export OUTPUTFOLDER; qsub -Wblock=true -N Encoder_run${RUN}_threads_${THREADS} -l select=1:ncpus=24:ompthreads=${THREADS}:mpiprocs=1:mem=4gb -o ${OUTPUTFOLDER}/run_${RUN}/enconding_result_${THREADS}_threads -e ${OUTPUTFOLDER}/run_${RUN}/enconding_result_${THREADS}_threads_err '/home/michele.yin/HuffmanCODEC/clusterscripts/parallelEncoderFileLocks.sh'; done;done;)