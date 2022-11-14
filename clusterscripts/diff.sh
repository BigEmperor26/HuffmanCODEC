
#!/bin/bash

#PBS -l select=1:ncpus=1:mem=2gb 
#PBS -l walltime=0:10:00
#PBS -q short_cpuQ
#PBS -o output_diff.out
#PBS -e errro_diff.out 
diff /home/michele.yin/HuffmanCODEC/data/movie/movie.mkv /home/michele.yin/HuffmanCODEC/data/movie/movie.mkv.huf.dec