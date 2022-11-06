
#!/bin/bash

#PBS -l select=1:ncpus=2:mem=2gb 
#PBS -l walltime=0:50:00
#PBS -q short_cpuQ
#PBS -o output_test.out
#PBS -e errro_test.out 
module load mpich-3.2 
module load openmpi-4.0.4
mpiexec -n 1 /home/michele.yin/HuffmanCODEC/bin/decode.out  /home/michele.yin/HuffmanCODEC/data/lorem.bin.huf /home/michele.yin/HuffmanCODEC/data/lorem.bin.dec