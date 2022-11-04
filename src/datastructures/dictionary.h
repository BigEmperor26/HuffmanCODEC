#ifndef __DICTIONARY__
#define __DICTIONARY__

#include "../commons/commons.h"

typedef struct Dictionary {
    ull frequencies[MAX_HEAP_SIZE];
    int size;
} Dictionary;


Dictionary* createDictionary(int size);

void freeDictionary(Dictionary* d);

void printDictionary(Dictionary* d);

#endif
