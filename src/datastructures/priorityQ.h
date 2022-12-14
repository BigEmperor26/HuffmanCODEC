#ifndef __PRIORITY_Q__
#define __PRIORITY_Q__

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "../commons/commons.h"

/*
** Priority queue implementation using the min heap data structure.
** The min heap is implemented as an array of nodes.
**
** Since the Huffman algorithm is going to deal with bytes,
** the priority queue max capacity is set to the number of
** different values a byte can assume.
*/


typedef struct Node {
    int value;  // 0-255 for data, -1 for placeholder
    ull priority;
    struct Node* left;
    struct Node* right;
} Node;

typedef struct PriorityQ {
    Node** minHeap;
    int size;
    int capacity;
} PriorityQ;

Node* createNode(int value, ull priority, Node* left, Node* right);
void freeNode(Node* node);

PriorityQ* createPriorityQ();
void freePriorityQ(PriorityQ* pq);

void printPriorityQ(PriorityQ* pq);

bool isEmpty(PriorityQ* pq);
bool isFull(PriorityQ* pq);

bool swapMinHeapElements(Node** minHeap, int index_a, int index_b);

bool pushPriorityQ(PriorityQ* pq, Node* node);
bool popPriorityQ(PriorityQ* pq, Node** node);
bool topPriorityQ(PriorityQ* pq, Node** node);


#endif
