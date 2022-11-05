#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../datastructures/priorityQ.h"
#include "../datastructures/dictionary.h"


ull get_frequencies(FILE* file, Dictionary* dict) {
    int	character;
    ull position;

    for (position = 0; ; position++) {
        character = fgetc(file);

        if (feof(file)) {
            break;
        }

        dict->frequencies[character]++;
    }
    return position;
}


void getHuffmanAlphabet(Node* node, int level, char currentCode[], char* alphabet[]) {

    if ((node->left == NULL) && (node->right == NULL)) {
        currentCode[level] = 0;  // end of string
        alphabet[node->value] = strdup(currentCode);
    }
    else {
        // left -> 0
        currentCode[level] = '0';
        getHuffmanAlphabet(node->left, level + 1, currentCode, alphabet);

        // right -> 1
        currentCode[level] = '1';
        getHuffmanAlphabet(node->right, level + 1, currentCode, alphabet);
    }
}


int main(int argc, char* argv[]) {

    Dictionary* dict = createDictionary(256);
    PriorityQ* pq = createPriorityQ();

    FILE* inputFile = fopen(argv[1], "r");
    ull fileSize = get_frequencies(inputFile, dict);
    fclose(inputFile);
    inputFile = NULL;


    // TODO: this operation takes O(nlogn), but it could take just O(n) with heapify
    for (int i = 0; i < dict->size; i++) {
        Node* node = createNode(i, dict->frequencies[i], NULL, NULL);
        pushPriorityQ(pq, node);
    }

    for (int i = 0; i < dict->size - 1; i++) {
        Node* child1;
        Node* child2;
        popPriorityQ(pq, &child1);
        popPriorityQ(pq, &child2);

        Node* newNode = createNode(-1, child1->priority + child2->priority, child1, child2);
        pushPriorityQ(pq, newNode);
    }

    Node* huffmanTree;
    popPriorityQ(pq, &huffmanTree);

    char* alphabet[MAX_HEAP_SIZE];
    char codeBuffer[MAX_HEAP_SIZE];

    getHuffmanAlphabet(huffmanTree, 0, codeBuffer, alphabet);

    for (int i=0; i<MAX_HEAP_SIZE; i++){
        printf("%d\t%s\n", i, alphabet[i]);
        free(alphabet[i]);
        alphabet[i] = NULL;
    }


    freeNode(huffmanTree);
    freeDictionary(dict);
    freePriorityQ(pq);

    huffmanTree = NULL;
    dict = NULL;
    pq = NULL;

    return 0;
}
