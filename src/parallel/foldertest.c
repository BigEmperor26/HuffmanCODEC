
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <stdbool.h>
#include <dirent.h>
#include <string.h>
// #include <linux/limits.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>

#ifdef _OPENMP 
# include <omp.h> 
#endif

#include "../datastructures/priorityQ.h"
#include "../commons/commons.h"

#define PATH_MAX 4096

/*
** function to count how many files are in a folder, recusively
*/
void countFiles(char* basePath, int* count) {
    char path[PATH_MAX];
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

/*
** Lists all files recursively at given path. Returns a list of files in files,
*/
void listFiles(char* basePath, int* current, char** files) {
    char path[PATH_MAX];
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

void fileSizeCounter(char** filenames, int files_count, int* file_sizes) {
    // file size, offset of each process, size to readof each process
    int file_size = 0;
    FILE* ptr;
    for (int i = 0;i < files_count;i++) {
        ptr = fopen(filenames[i], "rb");
        if (ptr == NULL) {
            printf("Error opening file %s\n", filenames[i]);
        }
        else {
            fseek(ptr, 0, SEEK_END); // seek to end of file
            file_size = ftell(ptr); // get current file pointer
            fclose(ptr);
        }
        file_sizes[i] = file_size;
    }
}

void filePreSorter(char** filenames, int* file_sizes, int files_count, char** new_filenames, int* new_file_sizes) {
    PriorityQ* files = createPriorityQ(files_count);
    for (int i = 0;i < files_count;i++) {
        Node* node = createNode(i, file_sizes[i], NULL, NULL);
        pushPriorityQ(files, node);
    }
    for (int i = files_count - 1;i >= 0;i--) {
        Node* node;
        popPriorityQ(files, &node);
        new_file_sizes[i] = node->priority;
        strcpy(new_filenames[i], filenames[node->value]);
        //printf("File %s with size %d\n",new_filenames[i],new_file_sizes[i]);
    }
    freePriorityQ(files);
}



int main(int argc, char** argv) {
    char inputFolder[PATH_MAX];
    strcpy(inputFolder, argv[1]);
    int files_count = 0;
    countFiles(inputFolder, &files_count);
    printf("files_count: %d \n", files_count);
    char** files = (char**)malloc(sizeof(char*) * files_count);
    for (int i = 0; i < files_count; i++) {
        files[i] = (char*)malloc(sizeof(char) * PATH_MAX);
    }

    int current = 0;
    listFiles(inputFolder, &current, files);
    int* file_sizes = (int*)malloc(sizeof(int) * files_count);
    fileSizeCounter(files, files_count, file_sizes);
    printf("Unsorted files\n");
    for (int i = 0; i < files_count; i++) {
        printf("file %s  size %d\n", files[i], file_sizes[i]);
    }
    char** sorted_files = malloc(sizeof(char*) * files_count);
    int* sorted_file_sizes = (int*)malloc(sizeof(int) * files_count);
    for (int i = 0;i < files_count;i++) {
        sorted_files[i] = (char*)malloc(sizeof(char) * PATH_MAX);
    }
    printf("Sorted files\n");
    filePreSorter(files, file_sizes, files_count, sorted_files, sorted_file_sizes);
    for (int i = 0; i < files_count; i++) {
        printf("file %s  size %d\n", sorted_files[i], sorted_file_sizes[i]);
    }
}