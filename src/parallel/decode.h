#ifndef __DECODE__
#define __DECODE__


#include "../datastructures/priorityQ.h"
#include "../datastructures/dictionary.h"
#include "huffman.h"
#include "frequency.h"


/*
** function to decode a bitstream recursively
*/
unsigned char getCharFromHuffmanEncodedBitStream(unsigned char buffer[], int* nbytes, int* nbits, Node* node);

bool getFrequenciesFromEncodedFile(FILE* inputFile, Dictionary* dict);
ull* getChunkOffsetsFromEncodedFile(FILE* inputFile, int* numChunks);
ull* getOriginalChunkSizesFromEncodedFile(FILE* inputFile, int* numChunks);

/*
** Function to decode a inputChunk to a outputChunk according to huffmanAlphabet
*/
bool chunkDecoder(unsigned char inputChunk[], unsigned char outputChunk[], Node* huffmanTree, ull inputChunkSize, ull* outputChunkSize);

/*
** Function to decode a input file to an output file according to given alphabet and chunk sizes
*/
bool fileDecoder(FILE* inputFile, FILE* outputFile, Node* huffmanTree, ull inputChunkOffsets[], ull inputChunkSizes[], int numOfChunks, int num_threads) ;

/*
** Function to perform end to end Huffman file decoding
*/
bool fileDecoderFull( char* inputFileName, char* outputFileName,int num_threads);

#endif