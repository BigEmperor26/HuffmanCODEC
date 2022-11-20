
#!/bin/bash

#PBS -l walltime=01:00:00 
#PBS -q short_cpuQ 
#PBS -v PROCESSES,THREADS,INPUT,INPUTFOLDER,OUTPUTFOLDER
#PBS -M michele.yin@studenti.unitn.it

INPUTFOLDER=${INPUTFOLDER%/}
OUTPUTFOLDER=${OUTPUTFOLDER%/}


MAIN=/home/michele.yin/HuffmanCODEC/bin/main.out
MODE=e
OUTPUT=${OUTPUTFOLDER%/}/${INPUT}_huf

TOTAL=$(($THREADS*$PROCESSES))

echo $(date)
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
time mpiexec --report-bindings -np ${PROCESSES} --map-by socket:pe=${THREADS} --bind-to core ${MAIN} -${MODE} -r -l ${INPUTFOLDER}/${INPUT} ${OUTPUT} #1> ${OUTPUTFOLDER}/enconding_result_${THREADS}_threads  2> ${OUTPUTFOLDER}/enconding_result_${THREADS}_threads_err

# How to run
# write SCRIPT='/home/michele.yin/HuffmanCODEC/clusterscripts/parallelEncoderFolder.sh'
# write INPUTFOLDER=path to folders to encode
# write OUTPUTFOLDER=path to specify where to save stdout and stderr
# write ENCODEPARSER=/home/michele.yin/HuffmanCODEC/src/results_averager_encode_folder.py
# then copy and paste this command to submit the job
# OUTPUTFOLDER will be created if it does not exist and it will contain the results of the for each run
# nohup $(for INPUT in $(ls $INPUTFOLDER); do for RUN in $(seq 1 3); do for PROCESSES in 1 2 4; do for THREADS in 1 2 4 6 8 10 12 16; do mkdir -p ${OUTPUTFOLDER}/${INPUT}/run_${RUN}; export PROCESSES; export THREADS; export INPUT;export INPUTFOLDER;export OUTPUTFOLDER; qsub -Wblock=true -N Encoder_run${RUN}_processes${PROCESSES}_threads_${THREADS} -l select=1:ncpus=$(($THREADS*$PROCESSES)):mpiprocs=${PROCESSES}:ompthreads=${THREADS}:mem=4gb -o ${OUTPUTFOLDER}/${INPUT}/run_${RUN}/enconding_result_${PROCESSES}_${THREADS}_threads -e ${OUTPUTFOLDER}/${INPUT}/run_${RUN}/enconding_result_${PROCESSES}_${THREADS}_threads_err ${SCRIPT}; done; done; done; done; python ${ENCODEPARSER} ${OUTPUTFOLDER}; );