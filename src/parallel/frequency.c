#include <omp.h>
#include "frequency.h"

#define NUM_THREADS 4
/*
** function that counts the chars in a chunk
*/ 
void countChunk(unsigned char *chunk,int size,Dictionary *d){
    for(int i = 0; i < size; i++){
        #pragma omp atomic
        d->frequencies[chunk[i]]++;
    }
}

/* 
** function that reads chunks from readfile, applies processChunk to each chunk, and writes the result in writefile 
*/
ull parallel_get_frequencies(FILE* file,Dictionary *d){

    fseek(file, 0, SEEK_END); // seek to end of file
    int file_size = ftell(file); // get current file pointer
    fseek(file, 0, SEEK_SET); // seek to end of file
    // chunks
    unsigned char chunk [NUM_THREADS][MAX_DECODED_BUFFER_SIZE];
    int chunk_size = MAX_DECODED_BUFFER_SIZE;
    int chunk_count = 0;
    chunk_count = file_size/chunk_size;
    if (file_size%chunk_size != 0){
        chunk_count++;
    }
    int chunk_iterations = chunk_count/NUM_THREADS;
    for(int i = 0; i < chunk_count; i+=NUM_THREADS){
        // sequential read of NUM_THREADS chunks
        int read[NUM_THREADS];
        int t_read = fread(chunk,sizeof(unsigned char),chunk_size*NUM_THREADS,file);
        for(int j=0;j<NUM_THREADS;j++){
            read[j]=0;
        }
        int j=0;
        while((t_read-chunk_size)>0){
            t_read = t_read-chunk_size;
            read[j] = chunk_size;
            j++;
        }
        read[j]=t_read;

        omp_set_dynamic(0); 
        omp_set_num_threads(NUM_THREADS); 
        #pragma omp parallel for
        for(int j=0;j<NUM_THREADS;j++){
            int thread_id = omp_get_thread_num();
            //  count the chunks
            countChunk(chunk[thread_id],read[j],d);
        }
    }
    return file_size;
}