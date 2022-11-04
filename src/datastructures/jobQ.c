#include "./jobQ.h"

JobQ* createJobQ(){
    JobQ* q = (JobQ*)malloc(sizeof(JobQ));
    q->size = 0;
    q->current = 0;
    q->head_chunks = NULL;
    q->tail_chunks = NULL;
    return q;
}

void pushJobQ(JobQ * d,Chunk * c){
    Chunk * temp = d->head_chunks;
    d->size++;
    return;
}
void popJobQ(JobQ * d,Chunk * c){
    
}
bool isEmptyJobQ(JobQ * d);

void freeJobQ(JobQ* d);

void printJobQ(JobQ* d);
