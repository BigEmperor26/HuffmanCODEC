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
ull parallel_get_frequencies(FILE* file,Dictionary *d){

    fseek(file, 0, SEEK_END); // seek to end of file
    int file_size = ftell(file); // get current file pointer
    fseek(file, 0, SEEK_SET); // seek to end of file
    // chunks
    unsigned char chunk [NUM_THREADS][MAX_DECODED_BUFFER_SIZE];
    int chunk_size = MAX_DECODED_BUFFER_SIZE;
    int chunk_count = 0;
    int read[NUM_THREADS];

    chunk_count = file_size/chunk_size;
    if (file_size%chunk_size != 0){
        chunk_count++;
    }
    int chunk_iterations = chunk_count/NUM_THREADS;
    if (chunk_count%NUM_THREADS != 0){
        chunk_iterations++;
    }
    omp_lock_t lock[NUM_THREADS];
    for (int j = 0;j < NUM_THREADS;j++) {
        omp_init_lock(&lock[j]);
        omp_set_lock(&lock[j]);
    }
    omp_set_dynamic(0); 
    omp_set_num_threads(NUM_THREADS); 
    #pragma omp parallel
    for(int i = 0; i < chunk_iterations; i++){
        // sequential read of NUM_THREADS chunks
        #pragma omp single
        {
            for(int j=0;j<NUM_THREADS;j++){
                read[j] = fread(chunk[j],sizeof(unsigned char),chunk_size,file);
                omp_unset_lock(&lock[j]);
            }
        }
        //#pragma omp barrier
        int thread_id = omp_get_thread_num();
        //  count the chunks
        omp_set_lock(&lock[thread_id]);
        if (read[thread_id]>0)
            countChunk(chunk[thread_id],read[thread_id],d);
        omp_unset_lock(&lock[thread_id]);
        #pragma omp barrier
        omp_set_lock(&lock[thread_id]);
    }
    return file_size;
}