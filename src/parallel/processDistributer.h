#ifndef __PROCESSDISTRIBUTER__
#define __PROCESSDISTRIBUTER__
#include "processDistributer.h"

/* function to distribute files_count files to size processes
** filenames is expectted to be a contiguous array of size files_count*PATH_MAX
** TO DO optimize the size of the output buffer
*/
void fileDistributer(char** filenames, int files_count, int rank, int size, char** files_to_process, int* files_to_process_count);


/*
** function to count the size of files in filenames, and return the size in file_sizes for each process
*/
void fileSizeCounter(char** filenames, int files_count, int rank, int* file_sizes);

#endif