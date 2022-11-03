
#ifndef __DICTIONARY__
#define __DICTIONARY__

#include <stdlib.h>
#include <stdbool.h>

#define MAX_HEAP_SIZE 256

typedef unsigned long long ull;

typedef struct Dictionary {
    ull frequency[MAX_HEAP_SIZE];
    int size;
} Dictionary;


Dictionary *  createDictionary(int size);

void freeDictionary(Dictionary * d);

void printDictionary(Dictionary * d);

#endif