
#!/bin/bash

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


time mpiexec -np ${PROCESSES} ${MAIN} -${MODE} -r -b ${INPUTFOLDER}/${INPUT} ${OUTPUT} #1> ${OUTPUTFOLDER}/enconding_result_${THREADS}_threads  2> ${OUTPUTFOLDER}/enconding_result_${THREADS}_threads_err

# How to run
# write SCRIPT='/home/michele.yin/HuffmanCODEC/clusterscripts/parallelEncoderFolder_local.sh'
# write INPUTFOLDER=path to folders to encode
# write OUTPUTFOLDER=path to specify where to save stdout and stderr
# write ENCODEPARSER=/home/michele.yin/HuffmanCODEC/src/results_averager_encode_folder.py
# then copy and paste this command to submit the job
# OUTPUTFOLDER will be created if it does not exist and it will contain the results of the for each run
# nohup $(for RUN in $(seq 1 3); do for PROCESSES in 1 2 4; do for THREADS in 1 ; do mkdir -p ${OUTPUTFOLDER}/${INPUT}/run_${RUN}; export PROCESSES; export THREADS; export INPUT;export OUTPUTFOLDER; $SCRIPT done; done; done; python ${ENCODEPARSER} ${OUTPUTFOLDER}; );