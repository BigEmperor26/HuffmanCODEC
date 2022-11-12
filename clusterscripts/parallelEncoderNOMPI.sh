
#!/bin/bash
#PBS -l walltime=00:10:00 
#PBS -q short_cpuQ 
#PBS -v THREADS,INPUT,OUTPUTFOLDER

MAIN=/home/michele.yin/HuffmanCODEC/bin/main.out

MODE=e
OUTPUT=${INPUT}.huf${THREADS}

echo ${MAIN}
echo ${MODE}
echo ${INPUT}
echo ${OUTPUT}
echo ${THREADS}
echo ${OUTPUTFOLDER}

module load mpich-3.2
module load openmpi-4.0.4
export OMP_NUM_THREADS=${THREADS}
time ${MAIN} -${MODE} ${INPUT} ${OUTPUT} 1> ${OUTPUTFOLDER}/enconding_result_${THREADS}_threads  2> ${OUTPUTFOLDER}/enconding_result_${THREADS}_threads_err

# { command 2>&1 1>&3 3>&- | stderr_command; } 3>&1 1>&2 | stdout_command
# How to run
# write THREADS=num for parallelism required
# write OUTPUTFOLDER=path to specify where to save stdout and stderr
# then copy and paste this command to submit the job
# export THREADS; export INPUT;export OUTPUTFOLDER; qsub '/home/michele.yin/HuffmanCODEC/clusterscripts/parallelEncoderNOMPI.sh'
# for THREADS in 1 2 4 6 10 12 16 20 24 32 48 64 96; do export THREADS; export INPUT;export OUTPUTFOLDER; qsub  -l select=1:ncpus=${THREADS}:mem=4gb '/home/michele.yin/HuffmanCODEC/clusterscripts/parallelEncoderNOMPI.sh'; done