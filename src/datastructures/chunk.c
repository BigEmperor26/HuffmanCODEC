
#include "../commons/commons.h"
#include "./chunk.h"

Chunk* createChunk(char * path,ull offset,ull size){
    Chunk* c = (Chunk*)malloc(sizeof(Chunk));
    strcpy(c->path,path);
    c->offset = offset;
    c->size = size;
    c->next = NULL;
    c->prev = NULL;
    return c;
}

void freeChunk(Chunk* d){
    free(d);
    return;
}

void printChunk(Chunk* d){
    printf("Path: %s\n",d->path);
    printf("Offset: %llu\n",d->offset);
    printf("Size: %llu\n",d->size);
    
    return;
}

