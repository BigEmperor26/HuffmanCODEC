#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

#include <mpi.h>
#ifdef _OPENMP 
# include <omp.h> 
#endif

#include "huffman.h"
#include "frequency.h"
#include "encode.h"
#include "../datastructures/priorityQ.h"
#include "../datastructures/dictionary.h"

/*
** Function to encode a inputChunk to a outputChunk according to huffmanAlphabet
*/
bool chunkEncoder( unsigned char* inputChunk,  unsigned char * outputChunk,char * huffmanAlphabet[],ull inputBufferSize,ull* outputChunkSize){
    int nbits=0;
    ull nbytes=0;
    unsigned char* currentCharHuffmanEncoded;
    ull currentCharHuffmanEncodedLength = 0;
    bool isEncodingSuccessful = true;
    // encode the chunk
    for (ull i = 0; i < inputBufferSize; i++) {
        currentCharHuffmanEncoded = (unsigned char*)huffmanAlphabet[inputChunk[i]];
        currentCharHuffmanEncodedLength = strlen((char*)currentCharHuffmanEncoded);
        for (ull j = 0; j < currentCharHuffmanEncodedLength; j++) {
            if (!fwriteBitInBuffer(currentCharHuffmanEncoded[j], (char*)outputChunk, &nbits, &nbytes)) {
                isEncodingSuccessful = false;
            }
        }
    }
    // add padding bits for the last byte of the output chunk
    while (nbits != 0) {
        if (!fwriteBitInBuffer('0', (char*)outputChunk, &nbits, &nbytes)) {
            isEncodingSuccessful = false;
        }
    }
    *outputChunkSize = nbytes;
    return isEncodingSuccessful;
}

/*
** Function to encode a file to an outputfile according huffmanAlphabet
*/
bool fileEncoderBarrier(FILE *inputFile,FILE* outputFile, char* huffmanAlphabet[],ull* outputFileSize,ull inputChunkSizes[], ull outputChunkSizes[],int num_threads){
    fseek(inputFile, 0, SEEK_END); // seek to end of file
    ull inputFileSize = ftell(inputFile); // get current file pointer
    fseek(inputFile, 0, SEEK_SET); // seek to start of file
    *outputFileSize = 0;
    // chunks
    unsigned char * inputChunk = (unsigned char*)malloc(sizeof(unsigned char)*num_threads*MAX_DECODED_BUFFER_SIZE);
    unsigned char * outputChunk = (unsigned char*)malloc(sizeof(unsigned char)*num_threads*(MAX_ENCODED_BUFFER_SIZE));
    // input
    ull chunkSize = MAX_DECODED_BUFFER_SIZE;
    // output variable length
    ull* inputBufferChunkSizes = malloc(sizeof(ull)*num_threads);
    // output variable length
    ull *outputBufferChunkSizes =  malloc(sizeof(ull)*num_threads); 
    ull numOfChunks = 0;
    // error detection
    bool isEncodingSuccessful = true;
    numOfChunks = inputFileSize/chunkSize;
    if (inputFileSize%chunkSize != 0){
        numOfChunks++;
    }
    ull chunkIterations = numOfChunks/num_threads;
    if (numOfChunks%num_threads != 0){
        chunkIterations++;
    }   
    #pragma omp parallel 
    for(ull i = 0; i < chunkIterations; i++){
        // reader
        #pragma omp single
        {
            for(int j=0;j<num_threads;j++){
                if (i*num_threads + j < numOfChunks){
                    inputBufferChunkSizes[j] = fread(inputChunk+j*MAX_DECODED_BUFFER_SIZE,sizeof(unsigned char),chunkSize,inputFile);
                    inputChunkSizes[i*num_threads+j] = inputBufferChunkSizes[j];
                }else{
                    inputBufferChunkSizes[j] = 0;
                }
            }
        }
        int thread_ID = 0;
        #ifdef _OPENMP 
            thread_ID = omp_get_thread_num();
        #endif
        if (inputBufferChunkSizes[thread_ID]>0){
            // Huffman compression of NUM_THREAD chunks
            isEncodingSuccessful = chunkEncoder(inputChunk+thread_ID*MAX_DECODED_BUFFER_SIZE,outputChunk+thread_ID*MAX_ENCODED_BUFFER_SIZE,huffmanAlphabet,inputBufferChunkSizes[thread_ID],&outputBufferChunkSizes[thread_ID]) && isEncodingSuccessful;
            if (i*num_threads + thread_ID < numOfChunks)
                outputChunkSizes[i*num_threads+thread_ID] = outputBufferChunkSizes[thread_ID];
        }else{
            outputBufferChunkSizes[thread_ID] = 0;
            if (i*num_threads + thread_ID < numOfChunks)
                outputChunkSizes[i*num_threads+thread_ID]=0;
        }
        #pragma omp barrier
        #pragma omp single
        {
            for(int j=0;j<num_threads;j++){
                if (inputBufferChunkSizes[j]>0){
                    fwrite(outputChunk+j*MAX_ENCODED_BUFFER_SIZE,sizeof(unsigned char),outputBufferChunkSizes[j],outputFile);
                    *outputFileSize +=outputBufferChunkSizes[j];
                }
            }
        }
    }
    free(inputChunk);
    free(outputChunk);
    free(inputBufferChunkSizes);
    free(outputBufferChunkSizes);
    
    return isEncodingSuccessful;
}

 /*
** Function to encode a file to an outputfile according huffmanAlphabet
*/

bool fileEncoderLocks(FILE *inputFile,FILE* outputFile, char* huffmanAlphabet[],ull* outputFileSize,ull inputChunkSizes[], ull outputChunkSizes[], int num_threads){
    fseek(inputFile, 0, SEEK_END); // seek to end of file
    ull inputFileSize = ftell(inputFile); // get current file pointer
    fseek(inputFile, 0, SEEK_SET); // seek to start of file
    *outputFileSize = 0;
    // chunks
    unsigned char * inputChunk = (unsigned char*)malloc(sizeof(unsigned char)*num_threads*MAX_DECODED_BUFFER_SIZE);
    unsigned char * outputChunk = (unsigned char*)malloc(sizeof(unsigned char)*num_threads*(MAX_ENCODED_BUFFER_SIZE));
    // input
    ull chunkSize = MAX_DECODED_BUFFER_SIZE;
    // output variable length
    ull* inputBufferChunkSizes = malloc(sizeof(ull)*num_threads);
    // output variable length
    ull *outputBufferChunkSizes =  malloc(sizeof(ull)*num_threads); 
    ull numOfChunks = 0;
    // error detection
    bool isEncodingSuccessful = true;
    numOfChunks = inputFileSize/chunkSize;
    if (inputFileSize%chunkSize != 0){
        numOfChunks++;
    }
    int chunkIterations = numOfChunks/num_threads;
    if (numOfChunks%num_threads != 0){
        chunkIterations++;
    }   
    // locks for multithreading
    // omp_lock_t readlock[num_threads];
    // omp_lock_t processlock[num_threads];
    // omp_lock_t writelock[num_threads];
    omp_lock_t * readlock = malloc(sizeof(omp_lock_t)*num_threads);
    omp_lock_t * processlock = malloc(sizeof(omp_lock_t)*num_threads);
    omp_lock_t * writelock = malloc(sizeof(omp_lock_t)*num_threads);
    for(int i=0;i<num_threads;i++){
        omp_lock_t r_lock, p_lock, w_lock;
        readlock[i] = r_lock;
        processlock[i] = p_lock;
        writelock[i] = w_lock;
    }
    int current_chunk = 0;
    for (int j = 0;j < num_threads;j++) {
        omp_init_lock(&readlock[j]);
        omp_init_lock(&processlock[j]);
        omp_init_lock(&writelock[j]);
        omp_unset_lock(&readlock[j]);
        omp_set_lock(&processlock[j]);
        omp_set_lock(&writelock[j]);
    }
    // set the parallel encoder
    omp_set_dynamic(0); 
    omp_set_num_threads(num_threads+2); 
    #pragma omp parallel
    for(int i = 0; i < chunkIterations; i++){
        int thread_ID = omp_get_thread_num();
        // single master thread as reader
        if(thread_ID ==0){
            for(int j=0;j<num_threads;j++){
                omp_set_lock(&readlock[j]);
                if (i*num_threads + j < numOfChunks){
                    inputBufferChunkSizes[j] = fread(inputChunk+j*MAX_DECODED_BUFFER_SIZE,sizeof(unsigned char),chunkSize,inputFile);
                    inputChunkSizes[i*num_threads+j] = inputBufferChunkSizes[j];
                }else{
                    inputBufferChunkSizes[j] = 0;
                }
                omp_unset_lock(&processlock[j]);
            }
        // single last thread as writer
        }else if(thread_ID==num_threads+1){
            for(int j=0;j<num_threads;j++){
                omp_set_lock(&writelock[j]);
                if (inputBufferChunkSizes[j]>0){
                    fwrite(outputChunk+j*MAX_ENCODED_BUFFER_SIZE,sizeof(unsigned char),outputBufferChunkSizes[j],outputFile);
                    *outputFileSize +=outputBufferChunkSizes[j];
                    outputChunkSizes[i*num_threads+j] = outputBufferChunkSizes[j];
                }
                omp_unset_lock(&readlock[j]);
            }
        // all other num_threads on processing
        }else{
            //double start_lock = omp_get_wtime();
            int work_ID = omp_get_thread_num()-1;
            #pragma omp critical
            {
                work_ID = current_chunk;
                current_chunk = (current_chunk +1)%num_threads;
            }
            // //test lock instead
            omp_set_lock(&processlock[work_ID]);
            //double start = omp_get_wtime();
            if (inputBufferChunkSizes[work_ID]>0){
                // Huffman compression of NUM_THREAD chunks
                isEncodingSuccessful = chunkEncoder(inputChunk+work_ID*MAX_DECODED_BUFFER_SIZE,outputChunk+work_ID*MAX_ENCODED_BUFFER_SIZE,huffmanAlphabet,inputBufferChunkSizes[work_ID],&outputBufferChunkSizes[work_ID]) && isEncodingSuccessful;
                // if (i*num_threads + work_ID < numOfChunks)
                //     outputChunkSizes[i*num_threads+work_ID] = outputBufferChunkSizes[work_ID];
            }else{
                outputBufferChunkSizes[work_ID] = 0;
                // if (i*num_threads + work_ID < numOfChunks)
                //     outputChunkSizes[i*num_threads+work_ID] = 0;
            }
            //double end = omp_get_wtime();
            // printf("Time required to process chunk %d for thread %d\n",i*num_threads+work_ID,thread_ID);
            // printf("Time since chunk acquired %f %d\n",end-start_lock,thread_ID);
            // printf("Time to process %f %d\n",end-start,thread_ID);
            omp_unset_lock(&writelock[work_ID]);
        }
    }
    for (int j = 0;j < num_threads;j++) {
        omp_destroy_lock(&readlock[j]);
        omp_destroy_lock(&processlock[j]);
        omp_destroy_lock(&writelock[j]);
    }
    free(inputChunk);
    free(outputChunk);
    free(inputBufferChunkSizes);
    free(outputBufferChunkSizes);
    free(readlock);
    free(processlock);
    free(writelock);
    return isEncodingSuccessful;
}


bool fileEncoderFull( char* inputFileName, char* outputFileName, int num_threads, int mode,int rank){
    clock_t start_freq = clock();
    double start_wall_freq = MPI_Wtime();
    // get byte frequencies in the input file
    FILE* inputFile = fopen(inputFileName, "r");
    if (!inputFile) {
        perror(inputFileName);
        exit(1);
    }

    Dictionary* dict = createDictionary(MAX_HEAP_SIZE);
    
    ull originalFileSize = parallel_get_frequencies(inputFile, dict,num_threads);
    clock_t end_freq = clock();
    double end_wall_freq = MPI_Wtime();
    printf("%d CPU Time to get frequencies %f\n",rank,((double) (end_freq - start_freq)) / CLOCKS_PER_SEC);
    printf("%d Wall Time to get frequencies %f\n",rank,end_wall_freq-start_wall_freq);
    clock_t start_tree = clock();
    double start_wall_tree = MPI_Wtime();
    // get huffman tree and alphabet
    char* huffmanAlphabet[MAX_HEAP_SIZE];
    char codeBuffer[MAX_HEAP_SIZE];
    Node* huffmanTree = getHuffmanTree(dict);
    getHuffmanAlphabet(huffmanTree, 0, codeBuffer, huffmanAlphabet);

    // prepare input and output files for encoding
    fseek(inputFile, 0, SEEK_SET);
    FILE* outputFile = fopen(outputFileName, "w");
    if (!outputFile) {
        perror(outputFileName);
        exit(1);
    }
    // encode file
    ull outputFileSize = 0;
    ull numOfChunks = (originalFileSize / MAX_DECODED_BUFFER_SIZE);
    if(originalFileSize % MAX_DECODED_BUFFER_SIZE != 0) 
       numOfChunks  += 1;
    ull* inputChunkSizes = (ull*)malloc(sizeof(ull) * numOfChunks);
    ull* outputChunkSizes = (ull*)malloc(sizeof(ull) * numOfChunks);

    clock_t end_tree = clock();
    double end_wall_tree = MPI_Wtime();
    double cpu_time_used_tree = ((double)(end_tree - start_tree)) / CLOCKS_PER_SEC;
    double wall_time_used_tree = end_wall_tree - start_wall_tree;
    printf("%d CPU Time required to only create the tree %f\n", rank,cpu_time_used_tree);
    printf("%d Wall Time required to only create the tree %f\n",rank, wall_time_used_tree);
    clock_t start = clock();
    double start_wall = MPI_Wtime();
    bool isEncodingSuccessful = false;
    if(mode==0)
        isEncodingSuccessful = fileEncoderBarrier(inputFile, outputFile, huffmanAlphabet, &outputFileSize, inputChunkSizes, outputChunkSizes,num_threads);
    else
        isEncodingSuccessful = fileEncoderLocks(inputFile, outputFile, huffmanAlphabet, &outputFileSize, inputChunkSizes, outputChunkSizes,num_threads);
    clock_t end = clock();
    double end_wall = MPI_Wtime();
    double cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    double wall_time_used = end_wall - start_wall;
    printf("%d CPU Time required to only encode %f\n",rank, cpu_time_used);
    printf("%d Wall Time required to only encode %f\n",rank, wall_time_used);
    printf("%s is %0.2f%% of %s\n", outputFileName, (float)outputFileSize / (float)originalFileSize, inputFileName);
    // write encoded file header footer:
    // - output chunk offsets
    
    ull firstOffset = 0;
    for (int i = 1; i < numOfChunks; i++) {
        outputChunkSizes[i] += outputChunkSizes[i-1];
    }

    fwrite(&firstOffset, sizeof(ull), 1, outputFile);
    fwrite(outputChunkSizes, sizeof(ull), numOfChunks, outputFile);
    // - input chunk sizes
    fwrite(inputChunkSizes, sizeof(ull), numOfChunks, outputFile);
    // - frequencies
    fwrite(dict->frequencies, sizeof(ull), MAX_HEAP_SIZE, outputFile);
    // - number of chunks
    fwrite(&numOfChunks, sizeof(ull), 1, outputFile);

    freeNode(huffmanTree);
    huffmanTree = NULL;

    freeDictionary(dict);
    dict = NULL;


    freeHuffmanAlphabet(huffmanAlphabet);

    free(inputChunkSizes);
    free(outputChunkSizes);

    clock_t start_cpu_flush = clock();
    double start_wall_flush = MPI_Wtime();

    // free resources
    fclose(inputFile);
    fclose(outputFile);

    clock_t end_cpu_flush = clock();
    double end_wall_flush = MPI_Wtime();
    double cpu_time_used_flush = ((double)(end_cpu_flush - start_cpu_flush)) / CLOCKS_PER_SEC;
    double wall_time_used_flush = end_wall_flush - start_wall_flush;
    printf("%d CPU Time required to only write flush %f\n",rank, cpu_time_used_flush);
    printf("%d Wall Time required to only write flush %f\n", rank,wall_time_used_flush);
    return isEncodingSuccessful;
    
}
