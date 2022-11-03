//  example of run "mpiexec -n 4 ./read test.bin"

#include <stdio.h>
#include <stdlib.h>
#include "priorityQ.h"
#include <mpi.h>

// int parseFileListFromFind(int readDescriptor, NamesList *fileList){
//     FILE *pipeToRead = fdopen(readDescriptor, "r");
//     int numOfFileNamesProcessed = 0;
//     char buffer[SIZE_OF_BUFFER_TO_READ_PIPE];
//     string filePath;

//     while(fgets(buffer, SIZE_OF_BUFFER_TO_READ_PIPE, pipeToRead) != NULL){
//         filePath = strtok(buffer, "\n");
//         absolutePath = realpath(filePath, absolutePath);
//         if(absolutePath != NULL){
//             appendNameToNamesList(fileList, absolutePath);
//         }
//         numOfFileNamesProcessed++;
//     }

//     return numOfFileNamesProcessed;
// }

// // Get all the file path inside a folder
// // Error codes:
// // 1: it was not possible to fork a child
// // 2: it was not possibile to inspect the folder
// int crawler(string folder, NamesList *fileList, int* outNumFilesFound){
//     int returnCode = 0;
//     *outNumFilesFound = -1; // in case of error;
//     string lsArgs[] = {"find", folder, "-type", "f", NULL};
//     int fds[2];
//     pipe(fds);

//     pid_t f = fork();
//     if (f < 0){
//         fprintf(stderr, "\nError creating little brittle crawler-son\n");
//         returnCode = 1;
//     } else if (f == 0){
//         // child
//         close(fds[READ]);
//         dup2(fds[WRITE], 1); // substitute stdout with fds[WRITE] for ls
//         execvp("find", lsArgs);
//         fprintf(stderr, "Error: it is not possibile to inspect folder: %s\n", folder);
//         returnCode = 2; // should never be here if exec works fine
//     } else {
//         // parent
//         close(fds[WRITE]);
        
//         // TODO check out == 0
//         int out; // return code from ls
//         pid_t pid = waitpid(f, &out, 0);
//         if (pid != -1){
//             *outNumFilesFound = parseFileListFromFind(fds[READ], fileList);
//         } else {
//             fprintf(stderr, "Error waiting for ls syscall termination\n");
//             returnCode = 3;
//         }
//     }

//     return returnCode;
// }
// // Given a path to a file/folder it returns:
// // -1 : if it does not exist
// //  0 : if it is a file and it exists
// //  1 : if it is is a folder and it exists
// int inspectPath(const char *path){
//     struct stat path_stat;
//     int returnCode = -1;
//     if (path != NULL && stat(path, &path_stat) == 0){
//         if (S_ISREG(path_stat.st_mode)){
//             returnCode = 0;
//         } else if (S_ISDIR(path_stat.st_mode)){
//             returnCode = 1;
//         }
//     }
//     return returnCode;
// }


// the key is the char as integer
// size is the number of different character or bytes, so 1 byte = 256
// the character is the key as integer, the frequency is the value
typedef struct Dictionary {
    ull frequency[MAX_HEAP_SIZE];
    int size;
} Dictionary;


Dictionary *  createDictionary(int size){
    Dictionary * d = (Dictionary*)malloc(sizeof(Dictionary));
    for(int i = 0; i < size; i++){
        d->frequency[i] = 0;
    }
    d->size = size;
    return d;
}

void freeDictionary(Dictionary * d){
    free(d);
    return;
}

void printDictionary(Dictionary * d){
    printf("Value\t");
    for (int i = 0; i < d->size; i++) {
        printf("%d ", i);
    }
    printf("\nFrequency\t");
    for (int i = 0; i < d->size; i++) {
        printf("%llu ", d->frequency[i]);
    }
    printf("\nSize: %d\n", d->size);
    return;
}

// with offset each partition is up to 2^64
bool charCounter(MPI_File *readfile, Dictionary *d, ull offset, ull size){
    // with binary offset
    char c[size];
    ull read = 0;
    MPI_Offset off_set = offset;
    if (MPI_File_read_at(*readfile,off_set, c, size, MPI_CHAR, MPI_STATUS_IGNORE) == MPI_SUCCESS){
        for(int i = 0; i < size; i++){
            d->frequency[c[i]]++;
        }
    }else{
        return false;
    }
    return true;
}

// function to count the frequency of each character in the file
// divides the file in 4 partitions and each process counts the frequency of each character in its partition
// if the file is smaller than the number of processes, only file size processes are active. The others are idle
bool charCounterProcess(char * filename,int rank, int size, Dictionary *d){
    // file size, offset of each process, size to readof each process
    int file_size = 0;
    int offset = 0;
    int size_to_read = 0;
    // if process 0, get the file size
    if(rank==0){
        //printf("World size is %d\n", size);
        FILE *ptr;
        ptr = fopen(filename,"rb"); 
        if (ptr == NULL){
            printf("Error opening file\n");
            return 1;
        }
        fseek(ptr, 0, SEEK_END); // seek to end of file
        file_size = ftell(ptr); // get current file pointer
        fclose(ptr);
        //printf("File size : %d\n", file_size);
    }
    // broadcast the file size to all processes
    MPI_Bcast(&file_size, 1, MPI_INT, 0, MPI_COMM_WORLD);
    // compute the offset for each process

    // if file size < number of processes, then only file size processes are needed
    int active_processes = size;
    if(file_size<size){
        active_processes = file_size;
    }
    if(rank<active_processes){
        offset = file_size/active_processes * rank;
        size_to_read = file_size/active_processes;
        if (rank == active_processes-1){
            size_to_read = file_size - offset;
        }
    }else{
        offset = 0;
        size_to_read = 0;
    }
    //printf("rank= %d, offset= %d, amount to read = %d \n", rank, offset, size_to_read);
    // for all processes, open the file
    MPI_File readfile;
    int rc = MPI_File_open(MPI_COMM_WORLD, filename ,MPI_MODE_RDONLY,MPI_INFO_NULL, &readfile);
    if(rc){
        printf("Error opening file\n");
        MPI_Abort(MPI_COMM_WORLD, rc);
    }
    // create a dictionary
    Dictionary * counts = createDictionary(256);
    // count the frequency of each character if the process is active
    if (rank<active_processes){
        charCounter(&readfile, counts, offset, size_to_read);
    }
    MPI_File_close(&readfile);
    // reduce the dictionary
    MPI_Reduce( (counts->frequency), d->frequency, 256, MPI_UNSIGNED_LONG_LONG, MPI_SUM, 0, MPI_COMM_WORLD);
    return true;
}


int main(int argc, char ** argv) {
    MPI_Init(&argc, &argv);

    char * filename = argv[1];
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    Dictionary * counts = createDictionary(256);
    charCounterProcess(filename, rank, size, counts);
    if(rank==0){
        printDictionary(counts);
    }
    freeDictionary(counts);
    MPI_Finalize();
    return 0;
}