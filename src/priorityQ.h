#include <stdio.h>
#include <stdlib.h>

// vector with data and priority
struct vectorTree {
    void** data; // data
    int* priority; // priority
    int size; // size
    int capacity; // capacity
};

// simple implementation of priority queue using binary max heap

// implemented using a vector because it's more efficient and easier to send using MPI

// costructor
struct vectorTree* createVectorTree(int capacity);

// destructor
void freeVectorTree(struct vectorTree* v);
// print the vector 
void printVectorTree(struct vectorTree* v);
// debug
void printHeight(struct vectorTree* v, int height);
// print the tree in a nice format
void printTree(struct vectorTree* v);
// check if empty
int empty(struct vectorTree* v);
// check if full
int full(struct vectorTree* v);
// return the current size
int size(struct vectorTree* v);

// debug
int intlog2(int x);
int intpow2(int x);
// swap two elements in the vector value and priority. Return 1 upon success. Undefined behavior if fails or indexes are out of bounds
int swap(void** value, int* priority, int index_a, int index_b);
// insert with priority the data in the vector. Return 1 upon success. 0 upon failure
int push(struct vectorTree* v, void* data, int priority);
// return the first element ( top of the heap ). Places result in data and priority. Returns 0 if failed. 1 if success
int top(struct vectorTree* v, void** data, int* priority);
// remove max priority. Places result in data and priority. Returns 0 if failed. 1 if success
int pop(struct vectorTree* v, void** data, int* priority);
