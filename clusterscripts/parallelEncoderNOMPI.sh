
#!/bin/bash
#PBS -l select=1:ncpus=1:mem=4gb 
#PBS -l walltime=00:40:00 
#PBS -q short_cpuQ 
#PBS -v THREADS,INPUT,FOLDER

MAIN=/home/michele.yin/HuffmanCODEC/bin/main.out
MODE=e
OUTPUT=${INPUT}.huf${THREADS}

echo ${MAIN}
echo ${INPUT}
echo ${OUTPUT}
echo ${MODE}
echo ${FOLDER}
echo ${THREADS}

module load mpich-3.2
module load openmpi-4.0.4
export OMP_NUM_THREADS=${THREADS}
time ${MAIN} -${MODE} ${FOLDER} ${INPUT} ${OUTPUT} 2>&1> ${INPUT}.enconding_result_${THREADS}_threads

# How to run
# write THREADS=num for parallelism required
# write FOLDER=-r if passing a folder as directory
# then copy and paste this command to submit the job
# export THREADS; export INPUT;export FOLDER; qsub '/home/michele.yin/HuffmanCODEC/clusterscripts/parallelEncoderNOMPI.sh'