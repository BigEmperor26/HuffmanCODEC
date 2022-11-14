#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef _OPENMP 
# include <omp.h> 
#endif

#include "frequency.h"

/*
** function that counts the chars in a chunk
*/ 
void countChunk(unsigned char *chunk,int size,Dictionary *d){
    for(int i = 0; i < size; i++){
        int value = chunk[i];
        #pragma omp atomic update
        d->frequencies[value]++;
    }
}

/* 
** function that reads chunks from readfile, applies processChunk to each chunk, and writes the result in writefile 
*/
ull parallel_get_frequencies(FILE* file,Dictionary *d, int num_threads){

    fseek(file, 0, SEEK_END); // seek to end of file
    int file_size = ftell(file); // get current file pointer
    fseek(file, 0, SEEK_SET); // seek to end of file
    // chunks
    unsigned char * chunk = (unsigned char*)malloc(sizeof(unsigned char)*num_threads*MAX_DECODED_BUFFER_SIZE);
    
    //unsigned char chunk [num_threads][MAX_DECODED_BUFFER_SIZE];
    int chunk_size = MAX_DECODED_BUFFER_SIZE;
    int chunk_count = 0;
    int * read = malloc(sizeof(int)*num_threads);
    //int read[num_threads];

    chunk_count = file_size/chunk_size;
    if (file_size%chunk_size != 0){
        chunk_count++;
    }
    int chunk_iterations = chunk_count/num_threads;
    if (chunk_count%num_threads != 0){
        chunk_iterations++;
    }
    // #ifdef _OPENMP 
    // omp_set_dynamic(0); 
    // omp_set_num_threads(num_threads); 
    // #endif
    #pragma omp parallel
    for(int i = 0; i < chunk_iterations; i++){

        int thread_ID = 0;
        #ifdef _OPENMP 
            thread_ID = omp_get_thread_num();
        #endif
        // master sequential read of num_threads chunks
        #pragma omp single
        {
            for(int j=0;j<num_threads;j++){
                read[j] = fread(chunk[j],sizeof(unsigned char),chunk_size,file);
            }
        }
        //  count the chunks
        if (read[thread_ID]>0)
            countChunk(chunk[thread_ID],read[thread_ID],d);
        
    }
    free(chunk);
    free(read);
    return file_size;
}