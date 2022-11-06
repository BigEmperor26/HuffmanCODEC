#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "huffman.h"
#include "../datastructures/priorityQ.h"
#include "../datastructures/dictionary.h"


/*
** Encode an input file to an output file using an huffman encoding alphabet
** Returns true in case of success, false otherwise.
*/
bool encodeOutputFile(FILE* inputFile, FILE* outputFile, char* huffmanAlphabet[], int* outputFileSize, ull chunkSizes[]) {
    unsigned char inputBuffer[MAX_DECODED_BUFFER_SIZE];
    unsigned char outputBuffer[MAX_ENCODED_BUFFER_SIZE];

    char* currentCharHuffmanEncoded;
    int currentCharHuffmanEncodedLength;

    int nbits, nbytes;
    *outputFileSize = 0;

    bool isEncodingSuccessful = true;
    int chunkCounter = 0;

    while (!feof(inputFile)) {
        // get of bytes from original decoded file
        int inputBufferSize = fread(inputBuffer, 1, MAX_DECODED_BUFFER_SIZE, inputFile);

        nbits = 0;
        nbytes = 0;

        chunkSizes[chunkCounter] = nbytes;
        chunkCounter++;

        // encode the chunk
        for (int i = 0; i < inputBufferSize; i++) {
            currentCharHuffmanEncoded = huffmanAlphabet[inputBuffer[i]];
            currentCharHuffmanEncodedLength = strlen(currentCharHuffmanEncoded);

            for (int i = 0; i < currentCharHuffmanEncodedLength; i++) {
                if (!fwriteBitInBuffer(currentCharHuffmanEncoded[i], outputFile, outputBuffer, &nbits, &nbytes, outputFileSize)) {
                    isEncodingSuccessful = false;
                }
            }
        }

        // add padding bits for the last byte of the output chunk
        while (nbits != 0) {
            if (!fwriteBitInBuffer('0', outputFile, outputBuffer, &nbits, &nbytes, outputFileSize)) {
                isEncodingSuccessful = false;
            }
        }

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
    char* outputFileName = (char*)malloc(strlen(inputFileName) + 4 + 1);
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

    int outputFileSize;
    int numOfChunks = (originalFileSize / MAX_DECODED_BUFFER_SIZE) + 1;
    ull* chunkSizes = (ull*)malloc(sizeof(ull) * numOfChunks);
    bool isEncodingSuccessful = encodeOutputFile(inputFile, outputFile, huffmanAlphabet, &outputFileSize, chunkSizes);
    printf("%s is %0.2f%% of %s\n", outputFileName, (float)outputFileSize / (float)originalFileSize, inputFileName);

    // write encoded file header footer:
    // - number of chunks
    ull offsetAccumulator = 0;
    fwrite(&numOfChunks, 1, sizeof(ull), outputFile);
    // - chunk offsets
    for (int i = 0; i < numOfChunks; i++) {
        chunkSizes[i] += offsetAccumulator;
        offsetAccumulator += chunkSizes[i];
    }
    fwrite(chunkSizes, numOfChunks, sizeof(ull), outputFile);
    fwrite(&offsetAccumulator, 1, sizeof(ull), outputFile);
    // - frequencies
    fwrite(dict->frequencies, MAX_HEAP_SIZE, sizeof(ull), outputFile);


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

    free(chunkSizes);

    return isEncodingSuccessful ? 0 : 1;
}
