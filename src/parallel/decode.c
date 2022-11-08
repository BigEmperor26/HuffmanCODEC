#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <mpi.h>
#include <omp.h>

#include "huffman.h"
#include "frequency.h"
#include "../datastructures/priorityQ.h"
#include "../datastructures/dictionary.h"


unsigned char getCharFromHuffmanEncodedBitStream(unsigned char buffer[], int* nbytes, int* nbits, Node* node) {

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
}

bool getFrequenciesFromEncodedFile(FILE* inputFile, Dictionary* dict) {
    fseek(inputFile, -(sizeof(ull) * (MAX_HEAP_SIZE + 1)), SEEK_END);

    int numFrequencies = fread(dict->frequencies, sizeof(ull), MAX_HEAP_SIZE, inputFile);

    return numFrequencies == MAX_HEAP_SIZE;
}

ull* getChunkOffsetsFromEncodedFile(FILE* inputFile, int* numChunks) {
    fseek(inputFile, -sizeof(ull), SEEK_END);

    fread(numChunks, sizeof(ull), 1, inputFile);

    ull* chunkOffsets = (ull*)malloc(sizeof(ull) * (*numChunks + 1));
    fseek(inputFile, -(sizeof(ull) * (MAX_HEAP_SIZE + 2 * *numChunks + 1 + 1)), SEEK_END);
    fread(chunkOffsets, sizeof(ull), *numChunks + 1, inputFile);

    return chunkOffsets;
}

ull* getOriginalChunkSizesFromEncodedFile(FILE* inputFile, int* numChunks) {
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
bool chunkDecoder(unsigned char inputChunk[], unsigned char outputChunk[], Node* huffmanTree, ull inputChunkSize, ull *outputChunkSize) {
    int nbytes = 0;
    int nbits = 0;
    ull outputCharCounter = 0;
    bool isDecodingSuccessful = true;
    // decode the chunk
    while (outputCharCounter <= inputChunkSize) {
        outputChunk[outputCharCounter] = getCharFromHuffmanEncodedBitStream(inputChunk, &nbytes, &nbits, huffmanTree);
        
        outputCharCounter++;
    }
    *outputChunkSize = outputCharCounter-1;
    return isDecodingSuccessful;
}

/*
** Function to decode a file to an outputfile according huffmanAlphabet
*/
bool fileDecoder(FILE* inputFile, FILE* outputFile, Node* huffmanTree, ull inputChunkOffsets[], ull inputChunkSizes[], int numOfChunks) {
    // chunks
    unsigned char inputChunk[NUM_THREADS][MAX_ENCODED_BUFFER_SIZE];
    // +1 to avoid overflow
    unsigned char outputChunk[NUM_THREADS][MAX_DECODED_BUFFER_SIZE+1];
    bool isDecodingSuccessful = true;
    ull inputBufferChunkSizes[NUM_THREADS];
    ull outputBufferChunkSizes[NUM_THREADS];
    ull decodedOutputBufferChunkSizes[NUM_THREADS];
    ull inputBufferChunkOffsets[NUM_THREADS];
    int chunkIterations = numOfChunks/NUM_THREADS;
    if (numOfChunks%NUM_THREADS != 0){
        chunkIterations++;
    }
    Node * huffTrees[NUM_THREADS];
    // locks for multithreading
    omp_lock_t lock[NUM_THREADS];
    for (int j = 0;j < NUM_THREADS;j++) {
        omp_init_lock(&lock[j]);
        omp_set_lock(&lock[j]);
    }
    omp_set_dynamic(0); 
    omp_set_num_threads(NUM_THREADS); 
    #pragma omp parallel
    for (int i = 0; i < chunkIterations; i++) {
        // single thread as reader
        #pragma omp single
        {
            ull readSize = 0;
            for (int j = 0;j < NUM_THREADS;j++) {
                if (i*NUM_THREADS + j < numOfChunks) {
                    inputBufferChunkSizes[j] = inputChunkOffsets[j + i*NUM_THREADS + 1] - inputChunkOffsets[j + i*NUM_THREADS];
                    outputBufferChunkSizes[j] = inputChunkSizes[j + i*NUM_THREADS];
                    inputBufferChunkOffsets[j] = inputChunkOffsets[j + i*NUM_THREADS];
                }
                else{
                    inputBufferChunkSizes[j] = 0;
                    outputBufferChunkSizes[j] = 0;
                    inputBufferChunkOffsets[j] = 0;
                }
                if (inputBufferChunkSizes[j] > 0) {
                    fseek(inputFile, inputBufferChunkOffsets[j], SEEK_SET);
                    readSize = fread(inputChunk[j], sizeof(unsigned char), inputBufferChunkSizes[j], inputFile);
                    if (readSize != inputBufferChunkSizes[j]) {
                        isDecodingSuccessful = false;
                    }
                }
                omp_unset_lock(&lock[j]);
            }

        }
        int thread_ID = omp_get_thread_num();
        omp_set_lock(&lock[thread_ID]);
        if (outputBufferChunkSizes[thread_ID] > 0) {
            chunkDecoder(inputChunk[thread_ID], outputChunk[thread_ID], huffmanTree, outputBufferChunkSizes[thread_ID],&decodedOutputBufferChunkSizes[thread_ID]);
            
        }
        omp_unset_lock(&lock[thread_ID]);
        
        #pragma omp barrier
        //writer
        #pragma omp single
        {
            for (int j = 0;j < NUM_THREADS;j++) {
                if (outputBufferChunkSizes[j] > 0) {
                    fwrite(outputChunk[j], sizeof(unsigned char), decodedOutputBufferChunkSizes[j], outputFile);
                }
                //omp_unset_lock(&lock[j]);
            }
        }
    }
    return isDecodingSuccessful;
}


int main(int argc, char* argv[]) {

    // initialize MPI
    int provided;
    MPI_Init_thread(&argc, &argv, MPI_THREAD_FUNNELED, &provided);
    // get the rank of the process
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    // get the number of processes
    int size;
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    // get input and output filenames
    char* inputFileName = argv[1];
    char* outputFileName = argv[2];

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
    int numChunks;
    ull* chunkOffsets = getChunkOffsetsFromEncodedFile(inputFile, &numChunks);
    ull* inputChunkSizes = getOriginalChunkSizesFromEncodedFile(inputFile, &numChunks);

    // prepare input and output files for encoding
    fseek(inputFile, 0, SEEK_SET);
    FILE* outputFile = fopen(outputFileName, "w");
    if (!outputFile) {
        perror(outputFileName);
        exit(1);
    }

    // decode
    bool isDecodingSuccessful = fileDecoder(inputFile, outputFile, huffmanTree, chunkOffsets, inputChunkSizes, numChunks);

    // free resources
    fclose(inputFile);
    fclose(outputFile);

    freeNode(huffmanTree);
    huffmanTree = NULL;

    freeDictionary(dict);
    dict = NULL;

    free(chunkOffsets);
    free(inputChunkSizes);

    MPI_Finalize();
    return  0;
}
