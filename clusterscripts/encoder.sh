
#!/bin/bash

#PBS -l select=1:ncpus=1:mem=2gb 
#PBS -l walltime=0:10:00
#PBS -q short_cpuQ
#PBS -o output_s1_encode.out
#PBS -e errro_s1_encode.out 
module load mpich-3.2 
module load openmpi-4.0.4
mpiexec -n 1 /home/michele.yin/HuffmanCODEC/bin/encode.out  /home/michele.yin/HuffmanCODEC/data/movie/movie.mkv 