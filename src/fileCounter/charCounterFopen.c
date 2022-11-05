//  example of compile mpicc -o charCounter ./fileCounter/charCounter.c ./datastructures/dictionary.c ./datastructures/priorityQ.c 
//  example of run mpiexec -n 1 ./charCounter ../data/256.bin  

#include "../commons/commons.h"
#include "../datastructures/dictionary.h"
#include "../datastructures/priorityQ.h"
#include <mpi.h>

// the key is the char as integer
// size is the number of different character or bytes, so 1 byte = 256
// the character is the key as integer, the frequency is the value


// in case, we can expand the char from 1 byte to 2 bytes 4 bytes or 8 bytes by using unsigned short, unsigned int, long, unsigned long long respectively

// with offset each partition is up to 2^64
bool charCounter(char *readfile, Dictionary *d, ull offset, ull size){
    // with binary offset
    u_int8_t c[size];
    FILE * ptr;
    ptr = fopen(readfile,"rb");
    if (ptr == NULL){
        printf("Error opening file\n");
        return 1;
    }
    fseek(ptr, offset, SEEK_SET);
    if (fread(c,1,size,ptr)){
        //#omp parallel for num_threads(4)5
        for(int i = 0; i < size; i++){
            d->frequencies[c[i]]++;
        }
        fclose(ptr);
    }else{
        fclose(ptr);
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
        }else{
            printf("File %s opened\n", filename);
        }
        fseek(ptr, 0, SEEK_END); // seek to end of file
        file_size = ftell(ptr); // get current file pointer
        fclose(ptr);
        printf("File size : %d\n", file_size);
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
    printf("rank= %d, offset= %d, amount to read = %d \n", rank, offset, size_to_read);
    
    // create a dictionary
    Dictionary * counts = createDictionary(MAX_HEAP_SIZE);
    // count the frequency of each character if the process is active
    if (rank<active_processes){
        charCounter(filename, counts, offset, size_to_read);
    }
    // reduce the dictionary
    MPI_Reduce( (counts->frequencies), d->frequencies, MAX_HEAP_SIZE, MPI_UNSIGNED_LONG_LONG, MPI_SUM, 0, MPI_COMM_WORLD);
    return true;
}
int main(int argc, char ** argv){
    // initialize MPI
    MPI_Init(&argc, &argv);
    // get the rank of the process
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    // get the number of processes
    int size;
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // create a dictionary
    Dictionary * d = createDictionary(MAX_HEAP_SIZE);
    // count the frequency of each character
    charCounterProcess(argv[1],rank,size,d);
    // if process 0, print the dictionary
    if(rank==0){
        printDictionary(d);
    }

    MPI_Finalize();
    return 0;
}