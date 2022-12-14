#ifndef __PROCESSDISTRIBUTER__
#define __PROCESSDISTRIBUTER__
#include "processDistributer.h"
#include "../commons/commons.h"

/* function to distribute files_count files to size processes
** filenames is expectted to be a contiguous array of size files_count*PATH_MAX
*/
void fileDistributer(char** filenames, int files_count, int rank, int size, char** files_to_process, int* files_to_process_count);


/*
** function to count the size of files in filenames, and return the size in file_sizes for each process
*/
void fileSizeCounter(char** filenames, int files_count, int rank, ull* file_sizes);

void filePreSorter(char** filenames, ull* file_sizes, int files_count, char** new_filenames, ull* new_file_sizes);
void fileSorterSize(char** filenames, ull* file_sizes, int files_count, int size, char** outputfilenames, int* indexes, int* files_per_process_count, ull* files_sizes_per_process);


void fileDistributerSize(char* filenames, int* process_indexes, int* file_per_process, int files_count, int rank, int size, char* files_to_process, int* files_to_process_count);
#endif