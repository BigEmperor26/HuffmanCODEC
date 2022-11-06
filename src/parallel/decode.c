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

#define NUM_THREADS 4

/*
** Function to read NUMTHREAD chunks from readFile, each of size in chunkSizes,
** according to the correct offset 
** Actual read chunk sizes are storead in read
*/
// // get chunk of bytes from encoded file
// chunkSize = chunkOffsets[indexChunk + 1] - chunkOffsets[indexChunk];
// fseek(inputFile, chunkOffsets[indexChunk], SEEK_SET);
// int inputBufferSize = fread(inputBuffer, 1, chunkSize, inputFile);

int chunksReader(FILE * file, unsigned char * outputChunk[],ull chunkOffsets[],int read[]){
    ull totalRead = 0;
    ull chunkSize = 0;
    for(int j=0;j<NUM_THREADS;j++){
        chunkSize = chunkOffsets[j + 1] - chunkOffsets[j];
        fseek(file, chunkOffsets[j], SEEK_SET);
        if (chunkSize>0){
            read[j] = fread(outputChunk[j],sizeof(unsigned char),chunkSize,file);
            totalRead +=read[j];
        }else{
            read[j]=0;
        }
    }
    return totalRead;
}


/*
** Function to read NUMTHREAD chunks from readFile, each of size chunkSize. 
** Actual read chunk sizes are storead in read
*/
int chunksWriter(FILE * file, char * outputChunk[],int* chunkSizes,int* written){
    int totalWritten = 0;
    for(int j=0;j<NUM_THREADS;j++){
        if (chunkSizes[j]>0){
            written[j] = fwrite(outputChunk[j],sizeof(unsigned char),chunkSizes[j],file);
            totalWritten +=written[j];
        }
    }
    return totalWritten;
}


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
bool chunkDecoder( char* inputChunk,  char * outputChunk,Node * huffmanTree,int inputChunkSize,int* outputChunkSize){
    int nbytes = 0;
    int nbits = 0;
    int outputCharCounter = 0;
    bool isDecodingSuccessful;

    if (inputChunkSize != inputChunkSize) {
            isDecodingSuccessful = false;
    }

    // decode the chunk
    while (outputCharCounter <= inputChunkSize) {
        outputChunk[outputCharCounter] = getCharFromHuffmanEncodedBitStream(inputChunk, &nbytes, &nbits, huffmanTree);
        outputCharCounter++;
    }

    return isDecodingSuccessful;
}

/*
** Function to decode a file to an outputfile according huffmanAlphabet
*/
bool fileDecoder(FILE *inputFile,FILE* outputFile, Node *huffmanTree,ull chunkOffsets[],ull inputChunkSizes[], int numOfChunks){
    // chunks
    unsigned char inputChunk[NUM_THREADS][MAX_ENCODED_BUFFER_SIZE];
    unsigned char outputChunk[NUM_THREADS][MAX_DECODED_BUFFER_SIZE];
            
    bool isDecodingSuccessful = true;
    
    ull inputBufferChunkSizes[NUM_THREADS];
    ull outputBufferChunkSizes[NUM_THREADS];
    for(int i = 0; i < numOfChunks; i+=NUM_THREADS){
        for(int j=0;j<NUM_THREADS;j++){
            if (i+j<numOfChunks)
                inputBufferChunkSizes[j] = inputChunkSizes[j+i];
            else
                inputBufferChunkSizes[j] = 0;
        }
        chunksReader(inputFile,(unsigned char**)inputChunk,chunkOffsets,inputBufferChunkSizes);
        // set the parallel decoder
        omp_set_dynamic(0); 
        omp_set_num_threads(NUM_THREADS); 
        #pragma omp parallel for
        for(int j=0;j<NUM_THREADS;j++){
            if (inputBufferChunkSizes[j]>0){
                isDecodingSuccessful = chunkDecoder(inputChunk,outputChunk,huffmanTree,inputBufferChunkSizes[j],&outputBufferChunkSizes[j]) && isDecodingSuccessful;
            }else{
                outputBufferChunkSizes[j] = 0;
            }
        }
        
        for(int j=0;j<NUM_THREADS;j++){
            if (outputBufferChunkSizes[j]>0){
                fwrite(outputChunk[j],sizeof(unsigned char),outputBufferChunkSizes[j],outputFile);
            }
        }

    }
   
    return isDecodingSuccessful;
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
    bool isDecodingSuccessful = decodeFile(inputFile, outputFile, huffmanTree, chunkOffsets, inputChunkSizes, numChunks);


    // int outputFileSize;
    // int numOfChunks = (originalFileSize / MAX_DECODED_BUFFER_SIZE) + 1;
    // ull* chunkSizes = (ull*)malloc(sizeof(ull) * numOfChunks);
    // bool isEncodingSuccessful = encodeOutputFile(inputFile, outputFile, huffmanAlphabet, &outputFileSize, chunkSizes);

    // printf("%s is %0.2f%% of %s\n", outputFileName, (float)outputFileSize / (float)originalFileSize, inputFileName);


    // free resources
    fclose(inputFile);
    fclose(outputFile);

    freeNode(huffmanTree);
    huffmanTree = NULL;

    freeDictionary(dict);
    dict = NULL;

    free(outputFileName);
    outputFileName = NULL;

    return isDecodingSuccessful ? 0 : 1;
}

