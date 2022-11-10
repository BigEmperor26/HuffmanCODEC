
#include <stdio.h>
#include <stdlib.h>
#include <linux/limits.h>
#include <mpi.h>
#include "processDistributer.h"

/* function to distribute files_count files to size processes
** filenames is expectted to be a contiguous array of size files_count*PATH_MAX
** TO DO optimize the size of the output buffer
*/
void fileDistributer(char** filenames, int files_count, int rank, int size, char** files_to_process, int* files_to_process_count) {
    // if file size < number of processes, then only file size processes are needed
    int active_processes = size;
    if (files_count < size) {
        active_processes = files_count;
    }
    int files_per_process_count[size];
    int displacements[size];
    for (int i = 0;i < size;i++) {
        if (i < active_processes) {
            files_per_process_count[i] = (files_count / active_processes) * PATH_MAX;
            displacements[i] = i * PATH_MAX;
        }else {
            files_per_process_count[i] = 0;
            displacements[i] = 0;
        }
        
        files_per_process_count[active_processes - 1] = (files_count - (files_count / active_processes) * (active_processes - 1)) * PATH_MAX;
        if (rank < active_processes) {
            MPI_Scatterv(filenames, files_per_process_count, displacements, MPI_CHAR, files_to_process, PATH_MAX * files_count, MPI_CHAR, 0, MPI_COMM_WORLD);
            *files_to_process_count = files_per_process_count[rank] / PATH_MAX;
        }
        else {
            MPI_Scatterv(filenames, 0, displacements, MPI_CHAR, files_to_process, 0, MPI_CHAR, 0, MPI_COMM_WORLD);
            *files_to_process_count = 0;
        }
    }
}


/*
** function to count the size of files in filenames, and return the size in file_sizes for each process
*/
void fileSizeCounter(char** filenames, int files_count, int rank, int* file_sizes) {
    // file size, offset of each process, size to readof each process
    int file_size = 0;
    FILE* ptr;
    for (int i = 0;i < files_count;i++) {
        ptr = fopen(filenames[i], "rb");
        if (ptr == NULL) {
            printf("Rank %d Error opening file %s\n", rank, filenames[i]);
        }
        else {
            fseek(ptr, 0, SEEK_END); // seek to end of file
            file_size = ftell(ptr); // get current file pointer
            fclose(ptr);
        }
        file_sizes[i] = file_size;
    }
}

