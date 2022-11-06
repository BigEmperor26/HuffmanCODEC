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
** Function to read NUMTHREAD chunks from readFile, each of size chunkSize. 
** Actual read chunk sizes are storead in read
*/
int chunksReader(FILE * file,unsigned char * outputChunk[],int chunkSize,int read[]){
    int totalRead = fread(outputChunk,sizeof(unsigned char),chunkSize*NUM_THREADS,file);
    for(int j=0;j<NUM_THREADS;j++){
        read[j]=0;
    }
    int j=0;
    while((totalRead-chunkSize)>0){
        totalRead = totalRead-chunkSize;
        read[j] = chunkSize;
        j++;
    }
    read[j]=totalRead;
    return totalRead;
}


/*
** Function to read NUMTHREAD chunks from readFile, each of size chunkSize. 
** Actual read chunk sizes are storead in read
*/
int chunksWriter(FILE * file, unsigned char * outputChunk[],int chunkSizes[]){
    int totalWritten = 0; 
    printf("writing \n");
    for(int j=0;j<NUM_THREADS;j++){  
        printf("writing chunks from %d of size %d\n",j,chunkSizes[j]);
    }
    for(int j=0;j<NUM_THREADS;j++){
        if (chunkSizes[j]>0){
            totalWritten += fwrite((unsigned char*) outputChunk[j],sizeof(unsigned char),chunkSizes[j],file);
        }
        printf("writing chunks from %d of size %d\n",j,chunkSizes[j]);
    }
    return totalWritten;
}

/*
** Function to encode a inputChunk to a outputChunk according to huffmanAlphabet
*/
bool chunkEncoder( unsigned char* inputChunk,  unsigned char * outputChunk,char * huffmanAlphabet[],int inputBufferSize,int* outputChunkSize){
    int nbits=0;
    int nbytes=0;
    unsigned char* currentCharHuffmanEncoded;
    int currentCharHuffmanEncodedLength = 0;
    bool isEncodingSuccessful = false;
    // encode the chunk
    for (int i = 0; i < inputBufferSize; i++) {
        currentCharHuffmanEncoded = huffmanAlphabet[inputChunk[i]];
        currentCharHuffmanEncodedLength = strlen(currentCharHuffmanEncoded);
        for (int i = 0; i < currentCharHuffmanEncodedLength; i++) {
            if (!fwriteBitInBuffer(currentCharHuffmanEncoded[i], outputChunk, &nbits, &nbytes)) {
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
bool fileEncoder(FILE *inputFile,FILE* outputFile, char* huffmanAlphabet[],int* outputFileSize,ull inputChunkSizes[], ull outputChunkSizes[]){
    fseek(inputFile, 0, SEEK_END); // seek to end of file
    int inputFileSize = ftell(inputFile); // get current file pointer
    fseek(inputFile, 0, SEEK_SET); // seek to start of file
    *outputFileSize = 0;
    // chunks
    unsigned char inputChunk[NUM_THREADS][MAX_DECODED_BUFFER_SIZE];
    unsigned char outputChunk[NUM_THREADS][MAX_ENCODED_BUFFER_SIZE];
    // input
    int chunkSize = MAX_DECODED_BUFFER_SIZE;
    // output variable length
    int inputBufferChunkSizes[NUM_THREADS];
    // output variable length
    int outputBufferChunkSizes[NUM_THREADS];
    int numOfChunks = 0;
    // error detection
    bool isEncodingSuccessful = true;
    numOfChunks = inputFileSize/chunkSize;
    if (inputFileSize%chunkSize != 0){
        numOfChunks++;
    }
    for(int i = 0; i < numOfChunks; i+=NUM_THREADS){
        chunksReader(inputFile,(unsigned char**)inputChunk,chunkSize,inputBufferChunkSizes);
        for(int j=0;j<NUM_THREADS;j++){
            inputChunkSizes[i+j] = inputBufferChunkSizes[j];
        }
        
        // set the parallel encoder
        omp_set_dynamic(0); 
        omp_set_num_threads(NUM_THREADS); 
        #pragma omp parallel for
        for(int j=0;j<NUM_THREADS;j++){
            if (inputBufferChunkSizes[j]>0){
                // Huffman compression of NUM_THREAD chunks
                isEncodingSuccessful = chunkEncoder((unsigned char*) inputChunk[j],(unsigned char*) outputChunk[j],huffmanAlphabet,inputBufferChunkSizes[j],&outputBufferChunkSizes[j]) && isEncodingSuccessful;
                outputChunkSizes[i+j] = outputBufferChunkSizes[j];
            }else{
                outputBufferChunkSizes[j] = 0;
            }
        }
        // bugfix TODO, 
        // questa funzione va in segfault
        //*outputFileSize += chunksWriter(outputFile,(unsigned char**)outputChunk,(int *)outputBufferChunkSizes);
        for(int j=0;j<NUM_THREADS;j++){
            if (outputBufferChunkSizes[j]>0){
                fwrite((unsigned char*) outputChunk[j],sizeof(unsigned char),outputBufferChunkSizes[j],outputFile);
                *outputFileSize +=outputBufferChunkSizes[j];
            }
            //printf("writing chunks from %d of size %d\n",j,outputBufferChunkSizes[j]);
        }

    }
   
    return isEncodingSuccessful;
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
    if (rank==0){
        printf("MPI initialized %d\n", provided);
    }

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
    ull originalFileSize = get_frequencies(inputFile, dict);
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
    bool isEncodingSuccessful = fileEncoder(inputFile,outputFile,huffmanAlphabet,&outputFileSize,inputChunkSizes,outputChunkSizes);
    printf("%s is %0.2f%% of %s\n", outputFileName, (float)outputFileSize / (float)originalFileSize, inputFileName);

    for(int j=0;j<numOfChunks;j++){
        printf("Read chunk sizes %d at %d\n",j,inputChunkSizes[j]);
    }
    for(int j=0;j<numOfChunks;j++){
        printf("Written chunk sizes %d at %d\n",j,outputChunkSizes[j]);
    }
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

    free(outputFileName);
    outputFileName = NULL;

    freeHuffmanAlphabet(huffmanAlphabet);

    free(inputChunkSizes);
    free(outputChunkSizes);

    MPI_Finalize(); 
    return isEncodingSuccessful ? 0 : 1;
    
}
