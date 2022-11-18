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
#include "decode.h"
#include "../datastructures/priorityQ.h"
#include "../datastructures/dictionary.h"


unsigned char getCharFromHuffmanEncodedBitStream(unsigned char buffer[], ull* nbytes, int* nbits, Node* node) {

    if (node->left == NULL && node->right == NULL) {
        return node->value;
    }

    bool isBitSet = buffer[*nbytes] & (1 << (7 - *nbits));

    (*nbits)++;
    if (*nbits == 8) {
        *nbits = 0;
        (*nbytes)++;
    }


    if (!isBitSet && node->left != NULL) {
        return getCharFromHuffmanEncodedBitStream(buffer, nbytes, nbits, node->left);
    }
    else if (node->right != NULL) {
        return getCharFromHuffmanEncodedBitStream(buffer, nbytes, nbits, node->right);
    }
    return '\0';
}

bool getFrequenciesFromEncodedFile(FILE* inputFile, Dictionary* dict) {
    fseek(inputFile, -(sizeof(ull) * (MAX_HEAP_SIZE + 1)), SEEK_END);

    int numFrequencies = fread(dict->frequencies, sizeof(ull), MAX_HEAP_SIZE, inputFile);

    return numFrequencies == MAX_HEAP_SIZE;
}

ull* getChunkOffsetsFromEncodedFile(FILE* inputFile, ull* numChunks) {
    fseek(inputFile, -sizeof(ull), SEEK_END);

    fread(numChunks, sizeof(ull), 1, inputFile);

    ull* chunkOffsets = (ull*)malloc(sizeof(ull) * (*numChunks + 1));
    fseek(inputFile, -(sizeof(ull) * (MAX_HEAP_SIZE + 2 * *numChunks + 1 + 1)), SEEK_END);
    fread(chunkOffsets, sizeof(ull), *numChunks + 1, inputFile);

    return chunkOffsets;
}

ull* getOriginalChunkSizesFromEncodedFile(FILE* inputFile, ull* numChunks) {
    fseek(inputFile, -sizeof(ull), SEEK_END);

    fread(numChunks, sizeof(ull), 1, inputFile);

    ull* chunkOffsets = (ull*)malloc(sizeof(ull) * (*numChunks));
    fseek(inputFile, -(sizeof(ull) * (MAX_HEAP_SIZE + *numChunks + 1)), SEEK_END);
    fread(chunkOffsets, sizeof(ull), *numChunks, inputFile);

    return chunkOffsets;
}

/*
** Function to decode a inputChunk to a outputChunk according to huffmanAlphabet
*/
bool chunkDecoder(unsigned char inputChunk[], unsigned char outputChunk[], Node* huffmanTree, ull inputChunkSize, ull* outputChunkSize) {
    ull nbytes = 0;
    int nbits = 0;
    ull outputCharCounter = 0;
    bool isDecodingSuccessful = true;
    // decode the chunk
    while (outputCharCounter <= inputChunkSize) {
        outputChunk[outputCharCounter] = getCharFromHuffmanEncodedBitStream(inputChunk, &nbytes, &nbits, huffmanTree);

        outputCharCounter++;
    }
    *outputChunkSize = outputCharCounter - 1;
    return isDecodingSuccessful;
}

/*
** Function to decode a file to an outputfile according huffmanAlphabet
*/
bool fileDecoderBarrier(FILE* inputFile, FILE* outputFile, Node* huffmanTree, ull inputChunkOffsets[], ull inputChunkSizes[], ull numOfChunks, int num_threads) {
    // chunks
    unsigned char * inputChunk = (unsigned char*)malloc(sizeof(unsigned char)*num_threads*MAX_ENCODED_BUFFER_SIZE);
    // +1 to avoid overflow
    unsigned char * outputChunk = (unsigned char*)malloc(sizeof(unsigned char)*num_threads*(MAX_DECODED_BUFFER_SIZE+1));
    bool isDecodingSuccessful = true;
    ull* inputBufferChunkSizes = (ull*)malloc(sizeof(ull)*num_threads);
    ull* outputBufferChunkSizes = (ull*)malloc(sizeof(ull)*num_threads);
    ull* decodedOutputBufferChunkSizes =(ull*) malloc(sizeof(ull)*num_threads);
    ull* inputBufferChunkOffsets = (ull*)malloc(sizeof(ull)*num_threads);
    ull chunkIterations = numOfChunks / num_threads;
    if (numOfChunks % num_threads != 0) {
        chunkIterations++;
    }
    #pragma omp parallel
    for (ull i = 0; i < chunkIterations; i++) {
        int thread_ID = 0;
        #ifdef _OPENMP 
            thread_ID = omp_get_thread_num();
        #endif
        // single master thread as reader
        #pragma omp single
        {
            ull readSize = 0;
            for (int j = 0;j < num_threads;j++) {
                if (i * num_threads + j < numOfChunks) {
                    inputBufferChunkSizes[j] = inputChunkOffsets[j + i * num_threads + 1] - inputChunkOffsets[j + i * num_threads];
                    outputBufferChunkSizes[j] = inputChunkSizes[j + i * num_threads];
                    inputBufferChunkOffsets[j] = inputChunkOffsets[j + i * num_threads];
                }
                else {
                    inputBufferChunkSizes[j] = 0;
                    outputBufferChunkSizes[j] = 0;
                    inputBufferChunkOffsets[j] = 0;
                }
                if (inputBufferChunkSizes[j] > 0) {
                    fseek(inputFile, inputBufferChunkOffsets[j], SEEK_SET);
                    readSize = fread(inputChunk+j*MAX_ENCODED_BUFFER_SIZE, sizeof(unsigned char), inputBufferChunkSizes[j], inputFile);
                    if (readSize != inputBufferChunkSizes[j]) {
                        isDecodingSuccessful = false;
                    }
                }
            }
        }
        if (outputBufferChunkSizes[thread_ID] > 0) {
            chunkDecoder(inputChunk+thread_ID*MAX_ENCODED_BUFFER_SIZE, outputChunk+thread_ID*(MAX_DECODED_BUFFER_SIZE+1), huffmanTree, outputBufferChunkSizes[thread_ID], &decodedOutputBufferChunkSizes[thread_ID]);
        }
        #pragma omp barrier
        #pragma omp single
        {
            // writer
            for (int j = 0;j < num_threads;j++) {
                if (outputBufferChunkSizes[j] > 0) {
                    fwrite(outputChunk+j*(MAX_DECODED_BUFFER_SIZE+1), sizeof(unsigned char), decodedOutputBufferChunkSizes[j], outputFile);
                }
            }
        }
    }
    free(inputChunk);
    free(outputChunk);
    free(inputBufferChunkSizes);
    free(outputBufferChunkSizes);
    free(decodedOutputBufferChunkSizes);
    free(inputBufferChunkOffsets);

    return isDecodingSuccessful;
}


/*
** Function to decode a file to an outputfile according huffmanAlphabet
*/
bool fileDecoderLocks(FILE* inputFile, FILE* outputFile, Node* huffmanTree, ull inputChunkOffsets[], ull inputChunkSizes[], ull numOfChunks, int num_threads) {
    unsigned char * inputChunk = (unsigned char*)malloc(sizeof(unsigned char)*num_threads*MAX_ENCODED_BUFFER_SIZE);
    // +1 to avoid overflow
    unsigned char * outputChunk = (unsigned char*)malloc(sizeof(unsigned char)*num_threads*(MAX_DECODED_BUFFER_SIZE+1));
    bool isDecodingSuccessful = true;
    ull* inputBufferChunkSizes = (ull*)malloc(sizeof(ull)*num_threads);
    ull* outputBufferChunkSizes = (ull*)malloc(sizeof(ull)*num_threads);
    ull* decodedOutputBufferChunkSizes =(ull*) malloc(sizeof(ull)*num_threads);
    ull* inputBufferChunkOffsets = (ull*)malloc(sizeof(ull)*num_threads);
    ull chunkIterations = numOfChunks / num_threads;
    if (numOfChunks % num_threads != 0) {
        chunkIterations++;
    }
    // locks for multithreading
    omp_lock_t readlock[num_threads];
    omp_lock_t processlock[num_threads];
    omp_lock_t writelock[num_threads];
    int current_chunk = 0;
    for (int j = 0;j < num_threads;j++) {
        omp_init_lock(&readlock[j]);
        omp_init_lock(&processlock[j]);
        omp_init_lock(&writelock[j]);
        omp_unset_lock(&readlock[j]);
        omp_set_lock(&processlock[j]);
        omp_set_lock(&writelock[j]);
    }
    omp_set_dynamic(0);
    omp_set_num_threads(num_threads+2); 
    #pragma omp parallel
    for (ull i = 0; i < chunkIterations; i++) {
        int thread_ID = omp_get_thread_num();
        // single master thread as reader
        if(thread_ID ==0){
            ull readSize = 0;
            for (ull j = 0;j < num_threads;j++) {
                omp_set_lock(&readlock[j]);
                if (i * num_threads + j < numOfChunks) {
                    inputBufferChunkSizes[j] = inputChunkOffsets[j + i * num_threads + 1] - inputChunkOffsets[j + i * num_threads];
                    outputBufferChunkSizes[j] = inputChunkSizes[j + i * num_threads];
                    inputBufferChunkOffsets[j] = inputChunkOffsets[j + i * num_threads];
                }
                else {
                    inputBufferChunkSizes[j] = 0;
                    outputBufferChunkSizes[j] = 0;
                    inputBufferChunkOffsets[j] = 0;
                }
                if (inputBufferChunkSizes[j] > 0) {
                    fseek(inputFile, inputBufferChunkOffsets[j], SEEK_SET);
                    readSize = fread(inputChunk+j*MAX_ENCODED_BUFFER_SIZE, sizeof(unsigned char), inputBufferChunkSizes[j], inputFile);
                    if (readSize != inputBufferChunkSizes[j]) {
                        isDecodingSuccessful = false;
                    }
                }
                omp_unset_lock(&processlock[j]);
            }
        // single last thread as writer
        }else if(thread_ID==num_threads+1){
            // writer
            for (int j = 0;j < num_threads;j++) {
                omp_set_lock(&writelock[j]);
                if (outputBufferChunkSizes[j] > 0) {
                    fwrite(outputChunk+j*(MAX_DECODED_BUFFER_SIZE+1), sizeof(unsigned char), decodedOutputBufferChunkSizes[j], outputFile);
                }
                omp_unset_lock(&readlock[j]);
            }
        }
        // all other num_threads on processing
        else{
            //double start_lock = omp_get_wtime();
            int work_ID = omp_get_thread_num()-1;
            #pragma omp critical
            {
                work_ID = current_chunk;
                current_chunk = (current_chunk + 1) % num_threads;
            }
            omp_set_lock(&processlock[work_ID]);
            //double start = omp_get_wtime();
            if (outputBufferChunkSizes[work_ID] > 0) {
                //sleep(1);
                chunkDecoder(inputChunk+work_ID*MAX_ENCODED_BUFFER_SIZE, outputChunk+work_ID*(MAX_DECODED_BUFFER_SIZE+1), huffmanTree, outputBufferChunkSizes[work_ID], &decodedOutputBufferChunkSizes[work_ID]);
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
    free(decodedOutputBufferChunkSizes);
    free(inputBufferChunkOffsets);

    return isDecodingSuccessful;
}

bool fileDecoderFull( char* inputFileName, char* outputFileName, int num_threads,int mode){
    clock_t start_tree = clock();
    double start_wall_tree = MPI_Wtime();
    // get byte frequencies in the input file
    FILE* inputFile = fopen(inputFileName, "r");
    if (!inputFile) {
        perror(inputFileName);
        exit(1);
    }

    Dictionary* dict = createDictionary(MAX_HEAP_SIZE);
    getFrequenciesFromEncodedFile(inputFile, dict);

    // get huffman tree and alphabet
    Node* huffmanTree = getHuffmanTree(dict);

    // get chunk offsets
    ull numChunks;
    ull* chunkOffsets = getChunkOffsetsFromEncodedFile(inputFile, &numChunks);
    ull* inputChunkSizes = getOriginalChunkSizesFromEncodedFile(inputFile, &numChunks);

    // prepare input and output files for encoding
    fseek(inputFile, 0, SEEK_SET);
    FILE* outputFile = fopen(outputFileName, "w");
    if (!outputFile) {
        perror(outputFileName);
        exit(1);
    }
    clock_t end_tree = clock();
    double end_wall_tree = MPI_Wtime();
    double cpu_time_used_tree = ((double)(end_tree - start_tree)) / CLOCKS_PER_SEC;
    double wall_time_used_tree = end_wall_tree - start_wall_tree;
    printf("CPU Time required to only create the tree %f\n", cpu_time_used_tree);
    printf("Wall Time required to only create the tree %f\n", wall_time_used_tree);
    // decode
    clock_t start = clock();
    double start_wall = MPI_Wtime();
    bool isDecodingSuccessful=false;
    if(mode==0)
        isDecodingSuccessful = fileDecoderBarrier(inputFile, outputFile, huffmanTree, chunkOffsets, inputChunkSizes, numChunks, num_threads);
    else
        isDecodingSuccessful = fileDecoderLocks(inputFile, outputFile, huffmanTree, chunkOffsets, inputChunkSizes, numChunks, num_threads);
    clock_t end = clock();
    double end_wall = MPI_Wtime();
    double cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    double wall_time_used = end_wall - start_wall;
    printf("CPU Time required to only decode %f\n", cpu_time_used);
    printf("Wall Time required to only decode %f\n", wall_time_used);

    // free resources
    fclose(inputFile);
    fclose(outputFile);

    freeNode(huffmanTree);
    huffmanTree = NULL;

    freeDictionary(dict);
    dict = NULL;

    free(chunkOffsets);
    free(inputChunkSizes);

    return  isDecodingSuccessful;
}
