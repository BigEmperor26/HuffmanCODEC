#ifndef __ENCODE__
#define __ENCODE__


#include "../datastructures/priorityQ.h"
#include "../datastructures/dictionary.h"
#include "huffman.h"
#include "frequency.h"

/*
** Function to encode a inputChunk to a outputChunk according to huffmanAlphabet
*/
bool chunkEncoder(unsigned char* inputChunk, unsigned char* outputChunk, char* huffmanAlphabet[], ull inputBufferSize, ull* outputChunkSize);

/*
** Function to encode a input file to an output file according to given alphabet and chunk sizes
*/
bool fileEncoderBarrier(FILE* inputFile, FILE* outputFile, char* huffmanAlphabet[], ull* outputFileSize, ull inputChunkSizes[], ull outputChunkSizes[], int num_threads);

#ifdef _OPENMP 
bool fileEncoderLocks(FILE* inputFile, FILE* outputFile, char* huffmanAlphabet[], ull* outputFileSize, ull inputChunkSizes[], ull outputChunkSizes[], int num_threads);
#endif

/*
** Function to perform end to end Huffman file encoding
*/
bool fileEncoderFull(char* inputFileName, char* outputFileName, int num_threads, int mode, int rank);

#endif