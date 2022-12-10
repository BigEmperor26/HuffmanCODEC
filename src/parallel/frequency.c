#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef _OPENMP 
# include <omp.h> 
#endif

#include "frequency.h"
#include "../commons/commons.h"

/*
** function that counts the chars in a chunk
*/ 
void countChunk(unsigned char *chunk,ull size,Dictionary *d){
    for(ull i = 0; i < size; i++){
        int value = chunk[i];
        //#pragma omp atomic update
        d->frequencies[value]++;
    }
}


/* 
** function that reads chunks from readfile, applies processChunk to each chunk, and writes the result in writefile 
*/
ull parallel_get_frequencies(FILE* file,Dictionary *d, int num_threads){

    Dictionary **dictionaries = (Dictionary**)malloc(num_threads*sizeof(Dictionary*));
    for(int i = 0; i < num_threads; i++){
        dictionaries[i] = createDictionary(MAX_HEAP_SIZE);
    }
    fseek(file, 0, SEEK_END); // seek to end of file
    ull file_size = ftell(file); // get current file pointer
    fseek(file, 0, SEEK_SET); // seek to end of file
    // chunks
    unsigned char * chunk = (unsigned char*)malloc(sizeof(unsigned char)*num_threads*MAX_DECODED_BUFFER_SIZE);
    
    ull chunk_size = MAX_DECODED_BUFFER_SIZE;
    ull chunk_count = 0;
    ull * read = malloc(sizeof(ull)*num_threads);

    chunk_count = file_size/chunk_size;
    if (file_size%chunk_size != 0){
        chunk_count++;
    }
    ull chunk_iterations = chunk_count/num_threads;
    if (chunk_count%num_threads != 0){
        chunk_iterations++;
    }
    #pragma omp parallel
    for(ull i = 0; i < chunk_iterations; i++){
        int thread_ID = 0;
        #ifdef _OPENMP 
            thread_ID = omp_get_thread_num();
        #endif
        // master sequential read of num_threads chunks
        #pragma omp single
        {
            for(int j=0;j<num_threads;j++){
                read[j] = fread(chunk+j*MAX_DECODED_BUFFER_SIZE,sizeof(unsigned char),chunk_size,file);
            }
        }
        //  count the chunks
        if (read[thread_ID]>0)
            countChunk(chunk+thread_ID*MAX_DECODED_BUFFER_SIZE,read[thread_ID],dictionaries[thread_ID]);
        
    }
    // merge dictionaries
    mergeDictionaries(d,dictionaries,num_threads);
    for(int i = 0; i < num_threads; i++){
        freeDictionary(dictionaries[i]);
    }
    free(dictionaries);
    free(chunk);
    free(read);
    return file_size;
}