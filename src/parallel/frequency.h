#ifndef __FREQUENCY__
#define __FREQUENCY__

#include "huffman.h"
#include "../datastructures/dictionary.h"

/*
** function that counts the chars in a chunk
*/ 
void countChunk(unsigned char *chunk,int size,Dictionary *d);

/* 
** function that reads chunks from readfile, applies processChunk to each chunk, and writes the result in writefile 
*/
ull parallel_get_frequencies(FILE* file,Dictionary *d);

#endif