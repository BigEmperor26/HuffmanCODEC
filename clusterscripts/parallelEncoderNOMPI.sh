
#!/bin/bash

#PBS -l select=1:ncpus=66:mem=4gb 
#PBS -l walltime=00:40:00
#PBS -q short_cpuQ
#PBS -o output_p_64_register_betterdiv_encode.out
#PBS -e errro_p_64_register_betterdiv_encode.out 
module load mpich-3.2
module load openmpi-4.0.4
time /home/michele.yin/HuffmanCODEC/bin/main.out -e -p 64 /home/michele.yin/HuffmanCODEC/data/movie/movie.mkv /home/michele.yin/HuffmanCODEC/data/movie/movie.mkv.huf64