#!/bin/bash

#PBS -l select=1:ncpus=1:mem=4gb
#PBS -l walltime=5:00:00
#PBS -q short_cpuQ

module load mpich-3.2
module load openmpi-4.0.4

/home/michele.yin/HuffmanCODEC/bin/data_generator.out