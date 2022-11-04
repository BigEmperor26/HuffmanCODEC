#ifndef __JOBQ__
#define __JOBQ__

#include "../commons/commons.h"
#include "./chunk.h"

typedef struct JobQ {
    Chunk * head_chunks;
    Chunk * tail_chunks;
    int current;
    int size;
} JobQ;


JobQ* createJobQ();

void pushJobQ(JobQ * d,Chunk * c);
void popJobQ(JobQ * d,Chunk * c);
bool isEmptyJobQ(JobQ * d);

void freeJobQ(JobQ* d);

void printJobQ(JobQ* d);

#endif
