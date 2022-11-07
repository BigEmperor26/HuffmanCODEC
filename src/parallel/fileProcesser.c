#include "../commons/commons.h"
#include "../datastructures/dictionary.h"
#include <mpi.h>
#include <omp.h>

// used only for cute strncpy in copyChunk
#include <string.h>



// empty function that processes a chunk of size 32*4096
// it just copies the chunk to processes_chunk
void copyChunk(char *chunk,char *processed_chunk){
    strncpy((char*)processed_chunk,(char*)chunk,32*4096);
}


// function that reads chunks from readfile, applies processChunk to each chunk, and writes the result in writefile 
bool fileProcesser(char *readfile,char* writefile,void (*processChunk)(char*,char*) ){
    FILE * readptr;
    readptr = fopen(readfile,"rb");
    if (readptr == NULL){
        printf("Error opening reader file %s\n", readfile);
        return 1;
    }
    printf("File reader %s opened\n", readfile);
    FILE * writeptr;
    writeptr = fopen(writefile,"wb");
    if (writeptr == NULL){
        printf("Error opening writer file %s\n", writefile);
        return 1;
    }
    printf("File writer %s opened\n", writefile);
    fseek(readptr, 0, SEEK_END); // seek to end of file
    int file_size = ftell(readptr); // get current file pointer
    fseek(readptr, 0, SEEK_SET); // seek to end of file
    printf("File size is %d\n", file_size);
    // chunks
    char chunk [4][32*4096];
    char processed_chunk [4][32*4096];
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
            read[j] = fread(chunk[j],sizeof(char),chunk_size,readptr);
            printf("current_read %d of chunk %d\n",read[j],j);
        }
        omp_set_dynamic(0); 
        omp_set_num_threads(NUM_THREADS); 
        #pragma omp parallel for
        for(int j=0;j<4;j++){
            int thread_id = omp_get_thread_num();
            // compress of decompress 4 chunks
            (*processChunk)(chunk[thread_id],processed_chunk[thread_id]);
        }
        int write[4];
        for(int j=0;j<4;j++){
            printf("current written chunk %s\n",chunk[j]);
            write[j] = fwrite(chunk[j],sizeof(char),read[j],writeptr);
            printf("current_written %d of chunk %d\n",write[j],j);
        }
    }
    fclose(readptr);
    fclose(writeptr);
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
    // process file in chunks and apply copyChunk
    fileProcesser(argv[1],argv[2],copyChunk);

    MPI_Finalize();
    return 0;
}