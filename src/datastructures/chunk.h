#ifndef __CHUNK__
#define __CHUNK__

#include "../commons/commons.h"

typedef struct Chunk {
    char path[MAX_PATH];
    ull offset;
    ull size;
    Chunk* next;
    Chunk* prev;
} Chunk;


Chunk* createChunk(char * path,ull offset,ull size);

void freeChunk(Chunk* d);

void printChunk(Chunk* d);

#endif
