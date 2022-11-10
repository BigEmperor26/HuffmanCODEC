#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <omp.h>
#include "frequency.h"

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
ull parallel_get_frequencies(FILE* file,Dictionary *d, int num_threads){

    fseek(file, 0, SEEK_END); // seek to end of file
    int file_size = ftell(file); // get current file pointer
    fseek(file, 0, SEEK_SET); // seek to end of file
    // chunks
    unsigned char chunk [num_threads][MAX_DECODED_BUFFER_SIZE];
    int chunk_size = MAX_DECODED_BUFFER_SIZE;
    int chunk_count = 0;
    int read[num_threads];

    chunk_count = file_size/chunk_size;
    if (file_size%chunk_size != 0){
        chunk_count++;
    }
    int chunk_iterations = chunk_count/num_threads;
    if (chunk_count%num_threads != 0){
        chunk_iterations++;
    }
    omp_lock_t readlock[num_threads];
    omp_lock_t processlock[num_threads];
    int current_chunk = 0;
    for (int j = 0;j < num_threads;j++) {
        omp_init_lock(&readlock[j]);
        omp_init_lock(&processlock[j]);
        omp_unset_lock(&readlock[j]);
        omp_set_lock(&processlock[j]);
    }
    omp_set_dynamic(0); 
    omp_set_num_threads(num_threads+1); 
    #pragma omp parallel
    for(int i = 0; i < chunk_iterations; i++){
        int thread_ID = omp_get_thread_num();
        // master sequential read of num_threads chunks
        if( thread_ID == 0){
            for(int j=0;j<num_threads;j++){
                omp_set_lock(&readlock[j]);
                read[j] = fread(chunk[j],sizeof(unsigned char),chunk_size,file);
                omp_unset_lock(&processlock[j]);
            }
        }else{
            int work_ID = 0;
            //int work_ID = omp_get_thread_num();
            #pragma omp critical
            {
                work_ID = current_chunk;
                current_chunk = (current_chunk +1)%num_threads;
            }
            //  count the chunks
            omp_set_lock(&processlock[work_ID]);
            if (read[work_ID]>0)
                countChunk(chunk[work_ID],read[work_ID],d);
            omp_unset_lock(&readlock[work_ID]);
        }
    }
    for (int j = 0;j < num_threads;j++) {
        omp_destroy_lock(&readlock[j]);
        omp_destroy_lock(&processlock[j]);
    }
    return file_size;
}