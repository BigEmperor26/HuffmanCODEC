#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "huffman.h"
#include "../commons/commons.h"
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


/*
** Encode an input file to an output file using an huffman encoding alphabet
** Returns true in case of success, false otherwise.
*/
bool decodeFile(FILE* inputFile, FILE* outputFile, Node* huffmanTree, ull chunkOffsets[], ull inputChunkSizes[], ull numChunks) {
    unsigned char inputBuffer[MAX_ENCODED_BUFFER_SIZE];
    unsigned char outputBuffer[MAX_DECODED_BUFFER_SIZE+1];

    ull nbytes;
    int nbits, chunkSize, outputCharCounter;

    bool isDecodingSuccessful = true;

    for (ull indexChunk = 0; indexChunk < numChunks; indexChunk++) {
        // get chunk of bytes from encoded file

        chunkSize = chunkOffsets[indexChunk + 1] - chunkOffsets[indexChunk];
        fseek(inputFile, chunkOffsets[indexChunk], SEEK_SET);
        int inputBufferSize = fread(inputBuffer, 1, chunkSize, inputFile);

        if (inputBufferSize != chunkSize) {
            isDecodingSuccessful = false;
        }
        nbytes = 0;
        nbits = 0;
        outputCharCounter = 0;

        // decode the chunk
        while (outputCharCounter < inputChunkSizes[indexChunk]) {
            outputBuffer[outputCharCounter] = getCharFromHuffmanEncodedBitStream(inputBuffer, &nbytes, &nbits, huffmanTree);
            outputCharCounter++;
        }
        // flush the buffer for the output chunk
        fwrite(outputBuffer, 1, outputCharCounter , outputFile);
    }

    return isDecodingSuccessful;
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
** Get the output filename adding a suffix to the input filename.
** The output filename is stored in the heap and can be freed with free()
*/
char* getOutputFileName(char* inputFileName) {
    // add .huf extension in the output file
    char* outputFileName = (char*)malloc(sizeof(char) * (strlen(inputFileName) + 4 + 1));
    sprintf(outputFileName, "%s.dec", inputFileName);

    return outputFileName;
}


int main(int argc, char* argv[]) {
    // get input and output filenames
    char* inputFileName = argv[1];
    char* outputFileName = getOutputFileName(inputFileName);

    // get byte frequencies in the input file
    FILE* inputFile = fopen(inputFileName, "rb");
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
    FILE* outputFile = fopen(outputFileName, "wb");
    if (!outputFile) {
        perror(outputFileName);
        exit(1);
    }

    // decode
    bool isDecodingSuccessful = decodeFile(inputFile, outputFile, huffmanTree, chunkOffsets, inputChunkSizes, numChunks);

    // free resources
    fclose(inputFile);
    fclose(outputFile);

    freeNode(huffmanTree);
    huffmanTree = NULL;

    freeDictionary(dict);
    dict = NULL;

    free(outputFileName);
    outputFileName = NULL;

    free(chunkOffsets);
    free(inputChunkSizes);
    return isDecodingSuccessful ? 0 : 1;
}
