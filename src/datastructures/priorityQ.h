#ifndef __PRIORITY_Q__
#define __PRIORITY_Q__

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#define MAX_HEAP_SIZE 256

/*
** Priority queue implementation using the min heap data structure.
** The min heap is implemented as an array of nodes.
**
** Since the Huffman algorithm is going to deal with bytes,
** the priority queue max capacity is set to the number of
** different values a byte can assume.
*/

typedef unsigned long long ull;

typedef struct Node {
    int value;  // 0-255 for data, -1 for placeholder
    ull priority;
    struct Node* left;
    struct Node* right;
} Node;

typedef struct PriorityQ {
    Node* minHeap[MAX_HEAP_SIZE];
    int size;
    int capacity;
} PriorityQ;

Node* createNode(int value, ull priority, Node* left, Node* right);

PriorityQ* createPriorityQ();
void freePriorityQ(PriorityQ* pq);

void printPriorityQ(PriorityQ* pq);

bool isEmpty(PriorityQ* pq);
bool isFull(PriorityQ* pq);

bool swapMinHeapElements(Node* minHeap[MAX_HEAP_SIZE], int index_a, int index_b);

bool pushPriorityQ(PriorityQ* pq, Node* node);
bool popPriorityQ(PriorityQ* pq, Node* node);
bool topPriorityQ(PriorityQ* pq, Node* node);


/*
void printHeight(struct vectorTree* v, int height);
// print the tree in a nice format
void printTree(struct vectorTree* v);

int intlog2(int x);
int intpow2(int x);
*/

#endif
