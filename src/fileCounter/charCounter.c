//  example of run "mpiexec -n 4 ./read test.bin"

#include "datastructures/priorityQ.h"
#include "datastructures/dictionary.h"
#include <mpi.h>

// the key is the char as integer
// size is the number of different character or bytes, so 1 byte = 256
// the character is the key as integer, the frequency is the value


// in case, we can expand the char from 1 byte to 2 bytes 4 bytes or 8 bytes by using unsigned short, unsigned int, long, unsigned long long respectively

// with offset each partition is up to 2^64
bool charCounter(MPI_File *readfile, Dictionary *d, ull offset, ull size){
    // with binary offset
    u_int8_t c[size];
    MPI_Offset off_set = offset;
    if (MPI_File_read_at_all(*readfile,off_set, c, size, MPI_UINT8_T, MPI_STATUS_IGNORE) == MPI_SUCCESS){
        for(int i = 0; i < size; i++){
            d->frequencies[c[i]]++;
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
    MPI_Reduce( (counts->frequencies), d->frequencies, 256, MPI_UNSIGNED_LONG_LONG, MPI_SUM, 0, MPI_COMM_WORLD);
    return true;
}