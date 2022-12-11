#ifndef __DECODE__
#define __DECODE__


#include "../datastructures/priorityQ.h"
#include "../datastructures/dictionary.h"
#include "huffman.h"
#include "frequency.h"


/*
** function to decode a bitstream recursively
*/
unsigned char getCharFromHuffmanEncodedBitStream(unsigned char buffer[], ull* nbytes, int* nbits, Node* node);

bool getFrequenciesFromEncodedFile(FILE* inputFile, Dictionary* dict);
ull* getChunkOffsetsFromEncodedFile(FILE* inputFile, ull* numChunks);
ull* getOriginalChunkSizesFromEncodedFile(FILE* inputFile, ull* numChunks);

/*
** Function to decode a inputChunk to a outputChunk according to huffmanAlphabet
*/
bool chunkDecoder(unsigned char inputChunk[], unsigned char outputChunk[], Node* huffmanTree, ull inputChunkSize, ull* outputChunkSize);

/*
** Function to decode a input file to an output file according to given alphabet and chunk sizes
*/
bool fileDecoderBarrier(FILE* inputFile, FILE* outputFile, Node* huffmanTree, ull inputChunkOffsets[], ull inputChunkSizes[], ull numOfChunks, int num_threads);

#ifdef _OPENMP 
bool fileDecoderLocks(FILE* inputFile, FILE* outputFile, Node* huffmanTree, ull inputChunkOffsets[], ull inputChunkSizes[], ull numOfChunks, int num_threads);
#endif


/*
** Function to perform end to end Huffman file decoding
*/
bool fileDecoderFull(char* inputFileName, char* outputFileName, int num_threads, int mode, int rank);

#endif