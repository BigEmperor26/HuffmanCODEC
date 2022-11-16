#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "huffman.h"
#include "../commons/commons.h"
#include "../datastructures/priorityQ.h"
#include "../datastructures/dictionary.h"


/*
** Encode an input file to an output file using an huffman encoding alphabet
** Returns true in case of success, false otherwise.
*/
bool encodeOutputFile(FILE* inputFile, FILE* outputFile, char* huffmanAlphabet[], ull* outputFileSize, ull inputChunkSizes[], ull outputChunkSizes[]) {
    unsigned char inputBuffer[MAX_DECODED_BUFFER_SIZE];
    unsigned char outputBuffer[MAX_ENCODED_BUFFER_SIZE];

    char* currentCharHuffmanEncoded;
    int currentCharHuffmanEncodedLength;

    int nbits;
    ull nbytes;
    *outputFileSize = 0;

    bool isEncodingSuccessful = true;
    int chunkCounter = 0;

    while (!feof(inputFile)) {
        // get of bytes from original decoded file
        int inputBufferSize = fread(inputBuffer, 1, MAX_DECODED_BUFFER_SIZE, inputFile);

        nbits = 0;
        nbytes = 0;

        // encode the chunk
        for (int i = 0; i < inputBufferSize; i++) {
            currentCharHuffmanEncoded = huffmanAlphabet[inputBuffer[i]];
            currentCharHuffmanEncodedLength = strlen(currentCharHuffmanEncoded);

            for (int i = 0; i < currentCharHuffmanEncodedLength; i++) {
                if (!fwriteBitInBuffer(currentCharHuffmanEncoded[i], outputFile, (char*)outputBuffer, &nbits, &nbytes, outputFileSize)) {
                    isEncodingSuccessful = false;
                }
            }
        }

        // add padding bits for the last byte of the output chunk
        while (nbits != 0) {
            if (!fwriteBitInBuffer('0', outputFile, (char*)outputBuffer, &nbits, &nbytes, outputFileSize)) {
                isEncodingSuccessful = false;
            }
        }

        inputChunkSizes[chunkCounter] = inputBufferSize;
        outputChunkSizes[chunkCounter] = nbytes;
        chunkCounter++;
        
        // flush the buffer for the output chunk
        fwrite(outputBuffer, 1, nbytes, outputFile);
    }

    return isEncodingSuccessful;
}

/*
** Get the output filename adding a suffix to the input filename.
** The output filename is stored in the heap and can be freed with free()
*/
char* getOutputFileName(char* inputFileName) {
    // add .huf extension in the output file
    char* outputFileName = (char*)malloc(sizeof(char) * (strlen(inputFileName) + 4 + 1));
    sprintf(outputFileName, "%s.huf", inputFileName);

    return outputFileName;
}


int main(int argc, char* argv[]) {
    // get input and output filenames
    char* inputFileName = argv[1];
    char* outputFileName = getOutputFileName(inputFileName);

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
    // printHuffmanAlphabet(huffmanAlphabet);

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

    ull outputFileSize;
    ull numOfChunks = (originalFileSize / MAX_DECODED_BUFFER_SIZE) + 1;
    ull* inputChunkSizes = (ull*)malloc(sizeof(ull) * numOfChunks);
    ull* outputChunkSizes = (ull*)malloc(sizeof(ull) * numOfChunks);
    bool isEncodingSuccessful = encodeOutputFile(inputFile, outputFile, huffmanAlphabet, &outputFileSize, inputChunkSizes, outputChunkSizes);
    printf("%s is %0.2f%% of %s\n", outputFileName, (float)outputFileSize / (float)originalFileSize, inputFileName);

    // write encoded file header footer:
    // - output chunk offsets
    ull firstOffset = 0;
    for (ull i = 1; i < numOfChunks; i++) {
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

    return isEncodingSuccessful ? 0 : 1;
}
