#include <stdio.h>
#include <stdlib.h>

// vector with data and priority
struct vectorTree{
    void **data; // data
    int *priority; // priority
    int size; // size
    int capacity; // capacity
};

// simple implementation of priority queue using binary max heap

// implemented using a vector because it's more efficient and easier to send using MPI

// costructor
struct vectorTree *createVectorTree(int capacity);

// destructor
void freeVectorTree(struct vectorTree *v);
// print the vector 
void printVectorTree(struct vectorTree *v);
// debug
void printHeight(struct vectorTree *v,int height);
// print the tree in a nice format
void printTree(struct vectorTree *v);
// check if empty
int empty(struct vectorTree *v);
// check if full
int full(struct vectorTree *v);
// return the current size
int size(struct vectorTree *v);

// debug
int intlog2(int x);
int intpow2(int x);
// swap two elements in the vector
int swap(void **value, int *priority, int index_a, int index_b);
// insert with priority
int push(struct vectorTree *v, void *data,int priority);
// return the first element ( top of the heap )
int top(struct vectorTree *v,void ** data, int*priority);
// remove max priority
int pop(struct vectorTree *v,void ** data, int *priority);
