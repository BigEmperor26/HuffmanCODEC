#include "../commons/commons.h"
#include "../datastructures/dictionary.h"
#include <mpi.h>
#include <omp.h>

// used only for cute strncpy in copyChunk
#include <string.h>

#define NUM_THREADS 4


// function that counts the chars in a chunk
void countChunk(unsigned char *chunk,int size,Dictionary *d){
    for(int i = 0; i < size; i++){
        #pragma omp atomic
        d->frequencies[chunk[i]]++;
    }
}


// function that reads chunks from readfile, applies processChunk to each chunk, and writes the result in writefile 
bool fileCounter(char *readfile ,Dictionary *d){
    FILE * readptr;
    readptr = fopen(readfile,"rb");
    if (readptr == NULL){
        printf("Error opening reader file %s\n", readfile);
        return 1;
    }
    printf("File reader %s opened\n", readfile);
   
    fseek(readptr, 0, SEEK_END); // seek to end of file
    int file_size = ftell(readptr); // get current file pointer
    fseek(readptr, 0, SEEK_SET); // seek to end of file
    printf("File size is %d\n", file_size);
    // chunks
    unsigned char chunk [4][4096];
    int chunk_size = 4096;
    int chunk_count = 0;
    chunk_count = file_size/chunk_size;
    if (file_size%chunk_size != 0){
        chunk_count++;
    }
    int chunk_iterations = chunk_count/4;
    printf("total chunks %d\n",chunk_count);
    for(int i = 0; i < chunk_count; i+=4){
        printf("chunks from %d to %d\n",i,i+4);
        // sequential read of 4 chunks
        int read[4];
        for(int j=0;j<4;j++){
            read[j] = fread(chunk[j],sizeof(unsigned char),chunk_size,readptr);
            printf("current_read %d of chunk %d\n",read[j],j);
        }
        omp_set_dynamic(0); 
        omp_set_num_threads(NUM_THREADS); 
        #pragma omp parallel for
        for(int j=0;j<4;j++){
            int thread_id = omp_get_thread_num();
            // compress of decompress 4 chunks
            countChunk(chunk[thread_id],read[j],d);
        }
    }
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
    fileCounter(argv[1],d);
    printDictionary(d);
    MPI_Finalize();
    return 0;
}