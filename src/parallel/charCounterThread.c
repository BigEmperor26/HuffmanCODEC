//  example of compile mpicc -o charCounter ./fileCounter/charCounter.c ./datastructures/dictionary.c ./datastructures/priorityQ.c 
//  example of run mpiexec -n 1 ./charCounter ../data/256.bin  

#include "../commons/commons.h"
#include "../datastructures/dictionary.h"
#include "../datastructures/priorityQ.h"
#include <mpi.h>
#include <omp.h>

#define NUM_THREADS 4
// the key is the char as integer
// size is the number of different character or bytes, so 1 byte = 256
// the character is the key as integer, the frequency is the value


// in case, we can expand the char from 1 byte to 2 bytes 4 bytes or 8 bytes by using unsigned short, unsigned int, long, unsigned long long respectively

// with offset each partition is up to 2^64
bool charCounter(char *readfile, Dictionary *d,char* writefile){
    FILE * readptr;
    readptr = fopen(readfile,"rb");
    if (readptr == NULL){
        printf("Error opening file %s\n", readfile);
        return 1;
    }
    printf("File %s opened\n", readfile);
    fseek(readptr, 0, SEEK_END); // seek to end of file
    int file_size = ftell(readptr); // get current file pointer
    fseek(readptr, 0, SEEK_SET); // seek to end of file
    printf("File size is %d\n", file_size);
    // chunks
    int chunk [4][32*4096];
    int chunk_size = 4096;
    int chunk_count = 0;
    chunk_count = file_size/chunk_size;
    if (file_size%chunk_size != 0){
        chunk_count++;
    }
    // for(int i = 0; i < chunk_count; i++){
    //     int current_read = fread(chunk[i],4,chunk_size,ptr);
    // } 
    // omp_set_dynamic(0);
    // omp_set_num_threads(NUM_THREADS); 
    
    omp_set_dynamic(0); 
    omp_set_num_threads(NUM_THREADS); 
    // Spawn 4 threads for this parallel region only
    #pragma omp parallel num_threads(4)
    {
        int ID = omp_get_thread_num();
        printf("Hello from thread %d\n", ID);
        int tot_threads = omp_get_num_threads();
        printf("Total threads %d\n", tot_threads);
    }
    // FILE * writeptr;
    // writeptr = fopen(writefile,"w");
    // if (writeptr == NULL){
    //     printf("Error creating file %s\n", writefile);
    //     return 1;
    // }
    
    fclose(readptr);
    return true;
}
int main(int argc, char ** argv){
    // initialize MPI
    int provided;
    MPI_Init_thread(&argc, &argv,MPI_THREAD_FUNNELED, &provided);

    // get the rank of the process
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    // get the number of processes
    int size;
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    if (rank==0){
        printf("MPI initialized %d\n", provided);
    }
    // create a dictionary
    Dictionary * d = createDictionary(MAX_HEAP_SIZE);
    // count the frequency of each character
    charCounter(argv[1],d,argv[2]);
    // if process 0, print the dictionary
    // if(rank==0){
    //     printDictionary(d);
    // }

    MPI_Finalize();
    return 0;
}