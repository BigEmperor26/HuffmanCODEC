#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <time.h>

//#include <mpi.h>
#include <omp.h>
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
    bool isEncodingSuccessful = false;
    // encode the chunk
    for (ull i = 0; i < inputBufferSize; i++) {
        currentCharHuffmanEncoded = huffmanAlphabet[inputChunk[i]];
        currentCharHuffmanEncodedLength = strlen(currentCharHuffmanEncoded);
        for (ull j = 0; j < currentCharHuffmanEncodedLength; j++) {
            if (!fwriteBitInBuffer(currentCharHuffmanEncoded[j], outputChunk, &nbits, &nbytes)) {
                isEncodingSuccessful = false;
            }
        }
    }
    // add padding bits for the last byte of the output chunk
    while (nbits != 0) {
        if (!fwriteBitInBuffer('0', outputChunk, &nbits, &nbytes)) {
            isEncodingSuccessful = false;
        }
    }
    *outputChunkSize = nbytes;
    return isEncodingSuccessful;
}

/*
** Function to encode a file to an outputfile according huffmanAlphabet
*/
bool fileEncoderBarrier(FILE *inputFile,FILE* outputFile, char* huffmanAlphabet[],int* outputFileSize,ull inputChunkSizes[], ull outputChunkSizes[],int num_threads){
    fseek(inputFile, 0, SEEK_END); // seek to end of file
    int inputFileSize = ftell(inputFile); // get current file pointer
    fseek(inputFile, 0, SEEK_SET); // seek to start of file
    *outputFileSize = 0;
    // chunks
    unsigned char inputChunk[num_threads][MAX_DECODED_BUFFER_SIZE];
    unsigned char outputChunk[num_threads][MAX_ENCODED_BUFFER_SIZE];
    // input
    ull chunkSize = MAX_DECODED_BUFFER_SIZE;
    // output variable length
    ull inputBufferChunkSizes[num_threads];
    // output variable length
    ull outputBufferChunkSizes[num_threads];
    int numOfChunks = 0;
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
    // set the parallel encoder
    // omp_set_dynamic(0); 
    // omp_set_num_threads(num_threads); 
    #pragma omp parallel 
    for(int i = 0; i < chunkIterations; i++){
        //double start = omp_get_wtime();
        // reader
        #pragma omp single
        {
            for(int j=0;j<num_threads;j++){
                if (i*num_threads + j < numOfChunks){
                    inputBufferChunkSizes[j] = fread(inputChunk[j],sizeof(unsigned char),chunkSize,inputFile);
                    inputChunkSizes[i*num_threads+j] = inputBufferChunkSizes[j];
                }else{
                    inputBufferChunkSizes[j] = 0;
                }
            }
        }
        //double start_work = omp_get_wtime();
        int thread_ID = 0;
        #ifdef _OPENMP 
            thread_ID = omp_get_thread_num();
        #endif
        // test lock instead
        if (inputBufferChunkSizes[thread_ID]>0){
            // Huffman compression of NUM_THREAD chunks
            isEncodingSuccessful = chunkEncoder(inputChunk[thread_ID],outputChunk[thread_ID],huffmanAlphabet,inputBufferChunkSizes[thread_ID],&outputBufferChunkSizes[thread_ID]) && isEncodingSuccessful;
            if (i*num_threads + thread_ID < numOfChunks)
                outputChunkSizes[i*num_threads+thread_ID] = outputBufferChunkSizes[thread_ID];
        }else{
            outputBufferChunkSizes[thread_ID] = 0;
            if (i*num_threads + thread_ID < numOfChunks)
                outputChunkSizes[i*num_threads+thread_ID]=0;
        }
        //double end_work = omp_get_wtime();
        //printf("Time required for thread %d on chunk %d %f\n",thread_ID,i*num_threads+thread_ID,end_work-start_work);
        #pragma omp barrier
        #pragma omp single
        {
            for(int j=0;j<num_threads;j++){
                if (inputBufferChunkSizes[j]>0){
                    fwrite(outputChunk[j],sizeof(unsigned char),outputBufferChunkSizes[j],outputFile);
                    *outputFileSize +=outputBufferChunkSizes[j];
                }
            }
        }
        //double end = omp_get_wtime();
        //printf("Time required for iteration %d %f\n",i,end-start);
    }
    return isEncodingSuccessful;
}

 /*
** Function to encode a file to an outputfile according huffmanAlphabet
*/

// bool fileEncoderLocks(FILE *inputFile,FILE* outputFile, char* huffmanAlphabet[],int* outputFileSize,ull inputChunkSizes[], ull outputChunkSizes[], int num_threads){
//     fseek(inputFile, 0, SEEK_END); // seek to end of file
//     int inputFileSize = ftell(inputFile); // get current file pointer
//     fseek(inputFile, 0, SEEK_SET); // seek to start of file
//     *outputFileSize = 0;
//     // chunks
//     unsigned char inputChunk[num_threads][MAX_DECODED_BUFFER_SIZE];
//     unsigned char outputChunk[num_threads][MAX_ENCODED_BUFFER_SIZE];
//     // input
//     ull chunkSize = MAX_DECODED_BUFFER_SIZE;
//     // output variable length
//     ull inputBufferChunkSizes[num_threads];
//     // output variable length
//     ull outputBufferChunkSizes[num_threads];
//     int numOfChunks = 0;
//     // error detection
//     bool isEncodingSuccessful = true;
//     numOfChunks = inputFileSize/chunkSize;
//     if (inputFileSize%chunkSize != 0){
//         numOfChunks++;
//     }
//     int chunkIterations = numOfChunks/num_threads;
//     if (numOfChunks%num_threads != 0){
//         chunkIterations++;
//     }   
//     // locks for multithreading
//     omp_lock_t readlock[num_threads];
//     omp_lock_t processlock[num_threads];
//     omp_lock_t writelock[num_threads];
//     int current_chunk = 0;
//     for (int j = 0;j < num_threads;j++) {
//         omp_init_lock(&readlock[j]);
//         omp_init_lock(&processlock[j]);
//         omp_init_lock(&writelock[j]);
//         omp_unset_lock(&readlock[j]);
//         omp_set_lock(&processlock[j]);
//         omp_set_lock(&writelock[j]);
//     }
//     // set the parallel encoder
//     omp_set_dynamic(0); 
//     omp_set_num_threads(num_threads+2); 
//     #pragma omp parallel
//     for(int i = 0; i < chunkIterations; i++){
//         int thread_ID = omp_get_thread_num();
//         // single master thread as reader
//         if(thread_ID ==0){
//             for(int j=0;j<num_threads;j++){
//                 omp_set_lock(&readlock[j]);
//                 if (i*num_threads + j < numOfChunks){
//                     inputBufferChunkSizes[j] = fread(inputChunk[j],sizeof(unsigned char),chunkSize,inputFile);
//                     inputChunkSizes[i*num_threads+j] = inputBufferChunkSizes[j];
//                 }else{
//                     inputBufferChunkSizes[j] = 0;
//                 }
//                 omp_unset_lock(&processlock[j]);
//             }
//         // single last thread as writer
//         }else if(thread_ID==num_threads+1){
//             for(int j=0;j<num_threads;j++){
//                 omp_set_lock(&writelock[j]);
//                 if (inputBufferChunkSizes[j]>0){
//                     fwrite(outputChunk[j],sizeof(unsigned char),outputBufferChunkSizes[j],outputFile);
//                     *outputFileSize +=outputBufferChunkSizes[j];
//                     outputChunkSizes[i*num_threads+j] = outputBufferChunkSizes[j];
//                 }
//                 omp_unset_lock(&readlock[j]);
//             }
//         // all other num_threads on processing
//         }else{
//             double start_lock = omp_get_wtime();
//             int work_ID = omp_get_thread_num()-1;
//             // #pragma omp critical
//             // {
//             //     work_ID = current_chunk;
//             //     current_chunk = (current_chunk +1)%num_threads;
//             // }
//             // #pragma omp atomic update
//             // work_ID = (current_chunk++)%num_threads;
//             //double start_lock = omp_get_wtime();
//             // test lock instead
//             omp_set_lock(&processlock[work_ID]);
//             double start = omp_get_wtime();
//             if (inputBufferChunkSizes[work_ID]>0){
//                 // Huffman compression of NUM_THREAD chunks
//                 isEncodingSuccessful = chunkEncoder(inputChunk[work_ID],outputChunk[work_ID],huffmanAlphabet,inputBufferChunkSizes[work_ID],&outputBufferChunkSizes[work_ID]) && isEncodingSuccessful;
//                 // if (i*num_threads + work_ID < numOfChunks)
//                 //     outputChunkSizes[i*num_threads+work_ID] = outputBufferChunkSizes[work_ID];
//             }else{
//                 outputBufferChunkSizes[work_ID] = 0;
//                 // if (i*num_threads + work_ID < numOfChunks)
//                 //     outputChunkSizes[i*num_threads+work_ID] = 0;
//             }
//             double end = omp_get_wtime();
//             printf("Time required to process chunk %d for thread %d\n",i*num_threads+work_ID,thread_ID);
//             printf("Time since chunk acquired %f %d\n",end-start_lock,thread_ID);
//             printf("Time to process %f %d\n",end-start,thread_ID);
//             omp_unset_lock(&writelock[work_ID]);
//         }
//     }
//     for (int j = 0;j < num_threads;j++) {
//         omp_destroy_lock(&readlock[j]);
//         omp_destroy_lock(&processlock[j]);
//         omp_destroy_lock(&writelock[j]);
//     }
//     return isEncodingSuccessful;
// }


bool fileEncoderFull( char* inputFileName, char* outputFileName, int num_threads){
    // get byte frequencies in the input file
    FILE* inputFile = fopen(inputFileName, "r");
    if (!inputFile) {
        perror(inputFileName);
        exit(1);
    }
    
    Dictionary* dict = createDictionary(MAX_HEAP_SIZE);
    ull originalFileSize = parallel_get_frequencies(inputFile, dict,num_threads);
    fclose(inputFile);
    inputFile = NULL;
    
    // get huffman tree and alphabet
    char* huffmanAlphabet[MAX_HEAP_SIZE];
    char codeBuffer[MAX_HEAP_SIZE];
    Node* huffmanTree = getHuffmanTree(dict);
    getHuffmanAlphabet(huffmanTree, 0, codeBuffer, huffmanAlphabet);

    // prepare input and output files for encoding
    inputFile = fopen(inputFileName, "r");
    FILE* outputFile = fopen(outputFileName, "w");
    if (!inputFile) {
        perror(inputFileName);
        exit(1);
    }
    if (!outputFile) {
        perror(outputFileName);
        exit(1);
    }

    // encode file
    int outputFileSize = 0;
    int numOfChunks = (originalFileSize / MAX_DECODED_BUFFER_SIZE) + 1;
    ull* inputChunkSizes = (ull*)malloc(sizeof(ull) * numOfChunks);
    ull* outputChunkSizes = (ull*)malloc(sizeof(ull) * numOfChunks);
    clock_t start = clock();
    bool isEncodingSuccessful = fileEncoderBarrier(inputFile, outputFile, huffmanAlphabet, &outputFileSize, inputChunkSizes, outputChunkSizes,num_threads);
    clock_t end = clock();
    double cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("CPU Time required to only encode %f\n", cpu_time_used);
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


    // free resources
    fclose(inputFile);
    fclose(outputFile);

    freeNode(huffmanTree);
    huffmanTree = NULL;

    freeDictionary(dict);
    dict = NULL;


    freeHuffmanAlphabet(huffmanAlphabet);

    free(inputChunkSizes);
    free(outputChunkSizes);

    return isEncodingSuccessful;
    
}
