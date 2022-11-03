#include "dictionary.h"

Dictionary* createDictionary(int size) {
    Dictionary* d = (Dictionary*)malloc(sizeof(Dictionary));
    for (int i = 0; i < size; i++) {
        d->frequency[i] = 0;
    }
    d->size = size;
    return d;
}

void freeDictionary(Dictionary* d) {
    free(d);
    return;
}

void printDictionary(Dictionary* d) {
    printf("Value\t");
    for (int i = 0; i < d->size; i++) {
        printf("%d ", i);
    }
    printf("\nFrequency\t");
    for (int i = 0; i < d->size; i++) {
        printf("%llu ", d->frequency[i]);
    }
    printf("\nSize: %d\n", d->size);
    return;
}
