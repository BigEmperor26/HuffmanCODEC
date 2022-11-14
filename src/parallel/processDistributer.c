
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <linux/limits.h>
#include <mpi.h>
#include "processDistributer.h"
#include "../datastructures/priorityQ.h"
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
void fileSizeCounter(char** filenames, int files_count, int rank, ull* file_sizes) {
    // file size, offset of each process, size to readof each process
    ull file_size = 0;
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


/* function to distribute the files according to the sizes of the files, using a priorityQ with priority the size of each file
** filenames is expectted to be a contiguous array of size files_count*PATH_MAX
*/
void fileSorterSize(char** filenames,ull * file_sizes, int files_count, int size,char ** outputfilenames, int * indexes, int * file_per_process_counts) {
    // if file size < number of processes, then only file size processes are needed
    int active_processes = size;
    if (files_count < size) {
        active_processes = files_count;
    }
    PriorityQ* pq = createPriorityQ();
    for (int i = 0;i < active_processes;i++) {
        Node * node = createNode(i,0,NULL,NULL);
        pushPriorityQ(pq, node);
    }
    char* process_files = (char*)malloc(sizeof(char)*active_processes*files_count*PATH_MAX); //char process_files[active_processes][files_count][PATH_MAX];
    int * indexes_per_process = (int*)malloc(sizeof(int)*active_processes); //int indexes_per_process[active_processes];
    for (int i = 0;i < active_processes;i++) {
        indexes_per_process[i] = 0;
    }
    for (int i = 0;i < files_count;i++) {
        Node* node;
        popPriorityQ(pq, &node);
        // assign next file to the process with the smallest size already assignes
        node->priority += file_sizes[i];
        int process = node->value;
        int index = indexes_per_process[process];
        strcpy(process_files + process*files_count*PATH_MAX + index*PATH_MAX, filenames[i]);
        //strcpy((char*)process_files[process][indexes_per_process[process]], (char*)filenames[i]);
        indexes_per_process[process]++;
        pushPriorityQ(pq, node);
    }
    int counter = 0;
    for (int i = 0;i < active_processes;i++) {
        indexes[i] = counter;
        file_per_process_counts[i] = indexes_per_process[i];
        for(int j=0;j<indexes_per_process[i];j++){
            //strcpy(outputfilenames + counter*PATH_MAX, process_files + i*files_count*PATH_MAX + j*PATH_MAX);
            //strcpy((char*)outputfilenames[counter], process_files[i][j]);
            strcpy((char*)outputfilenames[counter], process_files + i*files_count*PATH_MAX + j*PATH_MAX);
            counter++;
        }
    }   
    free(process_files);
    free(indexes_per_process);
}

void fileDistributerSize(char* filenames,int * process_indexes,int *file_per_process, int files_count, int rank, int size, char* files_to_process, int * files_to_process_count) {
    
    int displacements[size];
    int files_per_process_count[size];
    if(rank==0){
        for (int i = 0;i < size;i++) {
            displacements[i] = process_indexes[i]*PATH_MAX;
            files_per_process_count[i] = file_per_process[i]*PATH_MAX;
        }
    }
    MPI_Scatterv(filenames, files_per_process_count, displacements, MPI_CHAR, files_to_process, PATH_MAX * files_count, MPI_CHAR, 0, MPI_COMM_WORLD);
    MPI_Scatter(file_per_process, 1, MPI_INT, files_to_process_count, 1, MPI_INT, 0, MPI_COMM_WORLD);
}