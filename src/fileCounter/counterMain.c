//  example of run "mpiexec -n 4 ./read test.bin"

// every process needs to work on a list of chunks and they get redistributed to the processes

#include "../datastructures/priorityQ.h"
#include "../datastructures/dictionary.h"

#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <mpi.h>
#include <string.h>
//#include "charCounter.c"
#define MAX_PATH 2048

// function to count how many files are in a folder, recusively
void countFiles(char* basePath, int* count) {
    char path[MAX_PATH];
    struct dirent* dp;
    DIR* dir = opendir(basePath);
    // Unable to open directory stream
    if (!dir)
        return;
    while ((dp = readdir(dir)) != NULL) {
        if (strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0) {
            // if it is a file
            if (dp->d_type == DT_REG) {
                (*count)++;
            }
            else {
                // Construct new path from our base path
                strcpy(path, basePath);
                strcat(path, "/");
                strcat(path, dp->d_name);
                countFiles(path, count);
            }
        }
    }
    closedir(dir);
}

/**
 * Lists all files recursively at given path. Returns a list of files in files,
 */

void listFiles(char* basePath, int* current, char** files) {
    char path[MAX_PATH];
    struct dirent* dp;
    DIR* dir = opendir(basePath);
    // Unable to open directory stream
    if (!dir)
        return;
    while ((dp = readdir(dir)) != NULL) {
        if (strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0) {
            // if it is a file
            if (dp->d_type == DT_REG) {
                sprintf(files[*current], "%s/%s", basePath, dp->d_name);
                (*current)++;
            }
            // Construct new path from our base path
            strcpy(path, basePath);
            strcat(path, "/");
            strcat(path, dp->d_name);
            listFiles(path, current, files);
        }
    }
    closedir(dir);
}


// function to distribute files_count files to size processes
// filenames is expectted to be a contiguous array of size files_count*MAX_PATH
// TO DO optimize the size of the output buffer
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
            files_per_process_count[i] = (files_count / active_processes) * MAX_PATH;
            displacements[i] = i * MAX_PATH;
        }
        else {
            files_per_process_count[i] = 0;
            displacements[i] = 0;
        }
    }
    files_per_process_count[active_processes - 1] = (files_count - (files_count / active_processes) * (active_processes - 1)) * MAX_PATH;
    if (rank < active_processes) {
        MPI_Scatterv(filenames, files_per_process_count, displacements, MPI_CHAR, files_to_process, MAX_PATH * files_count, MPI_CHAR, 0, MPI_COMM_WORLD);
        *files_to_process_count = files_per_process_count[rank] / MAX_PATH;
    }
    else {
        MPI_Scatterv(filenames, 0, displacements, MPI_CHAR, files_to_process, 0, MPI_CHAR, 0, MPI_COMM_WORLD);
        *files_to_process_count = 0;
    }
}


// function to count the size of files in filenames, and return the size in file_sizes for each process
void fileSizeCounter(char** filenames, int files_count, int rank, int* file_sizes) {
    // file size, offset of each process, size to readof each process
    int file_size = 0;
    FILE* ptr;
    for (int i = 0;i < files_count;i++) {
        // printf("Weird suff\n");
        // printf("%s\n", filenames[i]);
        ptr = fopen(filenames[i], "rb");
        if (ptr == NULL) {
            printf("Rank %d Error opening file %s\n", rank, filenames[i]);
        }
        else {
            //printf("file %s was opened\n", filenames[i]);
            fseek(ptr, 0, SEEK_END); // seek to end of file
            file_size = ftell(ptr); // get current file pointer
            fclose(ptr);
        }
        file_sizes[i] = file_size;
    }
}

void fileChunker(char** filenames, int files_count, int rank, int* file_sizes){

}

// then get all the files
// count the sizes of each file
// divide the files in chunks according to parallelism

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    char* filename = argv[1];
    char* dirname = argv[2];
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);


    int files_count = 0;
    if (rank == 0) {
        countFiles(dirname, &files_count);
        printf("Number of files: %d\n", files_count);
    }

    MPI_Bcast(&files_count, 1, MPI_INT, 0, MPI_COMM_WORLD);

    char files[files_count][MAX_PATH];
    char* files_ptr[files_count];
    for (int i = 0;i < files_count;i++) {
        files_ptr[i] = files[i];
    }
    if (rank == 0) {
        int current = 0;
        listFiles(dirname, &current, (char**)files_ptr);
        printf("Files:\n");
        for (int i = 0;i < files_count;i++) {
            printf("%s\n", files[i]);
        }
    }
    char process_files[files_count][MAX_PATH];
    int process_count = 0;
    MPI_Barrier(MPI_COMM_WORLD);
    fileDistributer((char**)files, files_count, rank, size, (char**)process_files, &process_count);
    //printf("rank %d, process_count %d\n", rank, process_count);
    // for (int i = 0;i < process_count;i++) {
    //     printf("rank %d, file to process %s\n", rank, process_files[i]);
    // }
    int file_sizes[process_count];
    MPI_Barrier(MPI_COMM_WORLD);
    char* process_files_ptr[process_count];
    for (int i = 0;i < process_count;i++) {
        process_files_ptr[i] = process_files[i];
    }
    fileSizeCounter((char**)process_files_ptr, process_count, rank, file_sizes);
    MPI_Barrier(MPI_COMM_WORLD);
    // fileSizeCounter((char**)process_files,process_count,rank,file_sizes);
    for (int i = 0;i < process_count;i++) {
        printf("rank %d, file %s, size %d\n", rank, process_files[i], file_sizes[i]);
    }
    int total_size = 0;
    for (int i = 0;i < process_count;i++) {
        total_size += file_sizes[i];
    }
    int total_file_size = 0;
    MPI_Reduce(&total_size, &total_file_size, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        printf("Total size of files: %d\n", total_file_size);
    }

    // MPI_Bcast(&count, 1, MPI_INT, 0, MPI_COMM_WORLD);
    // MPI_Bcast(&current, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // //MPI_Bcast(*files, count, MPI_CHAR, 0, MPI_COMM_WORLD);
    // int* file_sizes = malloc(sizeof(int) * count);
    // fileSizeCounter(files, count, rank, size, file_sizes);

    MPI_Finalize();
    return 0;
}