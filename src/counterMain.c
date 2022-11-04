//  example of run "mpiexec -n 4 ./read test.bin"

#include "datastructures/priorityQ.h"
#include "datastructures/dictionary.h"
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <mpi.h>
#include <string.h>
#include "charCounter.c"
#define MAX_PATH 2048

// function to count how many files are in a folder, recusively
void countFiles(char* basePath, int* count) {
    char path[MAX_PATH];
    struct dirent* dp;
    DIR* dir = opendir(basePath);
    // Unable to open directory stream
    if (!dir)
        return;

    while ((dp = readdir(dir)) != NULL)
    {
        if (strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0)
        {
            // if it is a file
            if (dp->d_type == DT_REG) {
                //printf("%s/%s\n", basePath, dp->d_name);
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
 * Lists all files recursively at given path.
 */

void listFiles(char* basePath, int* current, char** files) {
    char path[MAX_PATH];
    struct dirent* dp;
    DIR* dir = opendir(basePath);

    // Unable to open directory stream
    if (!dir)
        return;

    while ((dp = readdir(dir)) != NULL)
    {
        if (strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0)
        {
            // if it is a file
            if (dp->d_type == DT_REG) {
                files[*current] = malloc(sizeof(char) * MAX_PATH);
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

void fileSizeCounter(char** filenames, int files_count, int rank, int size, int* file_sizes) {
    // file size, offset of each process, size to readof each process
    int file_size = 0;
    FILE* ptr;
    int file_index_start = files_count / size * rank;
    int files_per_process = files_count / size;
    if (rank == size - 1) {
        files_per_process = files_count - file_index_start;
    }
    printf("rank %d, file_index_start %d, files_per_process %d\n", rank, file_index_start, files_per_process);
    for (int i = 0;i < files_per_process;i++) {
        ptr = fopen(filenames[file_index_start + i], "rb");
        if (ptr == NULL) {
            printf("Error opening file %s\n", filenames[file_index_start + i]);
        
        }else{
            fseek(ptr, 0, SEEK_END); // seek to end of file
            file_size = ftell(ptr); // get current file pointer
            fclose(ptr);
        }
        
        //file_sizes[file_index_start+i] = file_size;

    }
    else {
        offset = 0;
        size_to_read = 0;
    }
    //printf("rank= %d, offset= %d, amount to read = %d \n", rank, offset, size_to_read);
    // for all processes, open the file
    MPI_File readfile;
    int rc = MPI_File_open(MPI_COMM_WORLD, filename, MPI_MODE_RDONLY, MPI_INFO_NULL, &readfile);
    if (rc) {
        printf("Error opening file\n");
        MPI_Abort(MPI_COMM_WORLD, rc);
    }
    // create a dictionary
    Dictionary* counts = createDictionary(MAX_HEAP_SIZE);
    // count the frequency of each character if the process is active
    if (rank < active_processes) {
        charCounter(&readfile, counts, offset, size_to_read);
    }
    MPI_File_close(&readfile);
    // reduce the dictionary
    MPI_Reduce(counts->frequencies, d->frequencies, MAX_HEAP_SIZE, MPI_UNSIGNED_LONG_LONG, MPI_SUM, 0, MPI_COMM_WORLD);
    // free the dictionary
    freeDictionary(counts);
    return true;
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

    Dictionary* counts = createDictionary(MAX_HEAP_SIZE);
    charCounterProcess(filename, rank, size, counts);

    int count = 0;
    char** files = malloc(sizeof(char*) * count);
    int current = 0;

    if (rank == 0) {
        countFiles(dirname, &count);
        printf("Number of files: %d\n", count);
        listFiles(dirname, &current, files);
        for (int i = 0;i < count;i++) {
            printf("%s", files[i]);
        }

    }
    // FILE * ptr = fopen("../../simple_exercises/huffman/pentagon.c", "rb");
    // if (ptr == NULL) {
    //     printf("Error opening file %s\n", "../../simple_exercises/huffman/pentagon.c");
    //     return 1;
    // }else{
    //     printf("File opened successfully %s\n", "../../simple_exercises/huffman/pentagon.c");
    // }
    MPI_Bcast(&count, 1, MPI_INT, 0, MPI_COMM_WORLD);
    //MPI_Bcast(*files, count, MPI_CHAR, 0, MPI_COMM_WORLD);
    MPI_Bcast(&current, 1, MPI_INT, 0, MPI_COMM_WORLD);
    int* file_sizes = malloc(sizeof(int) * count);
    fileSizeCounter(files, count, rank, size, file_sizes);

    freeDictionary(counts);
    MPI_Finalize();
    return 0;
}