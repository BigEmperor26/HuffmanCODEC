#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "huffman.h"
#include "../commons/commons.h"

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


Node* getHuffmanTree(Dictionary* dict){
    PriorityQ* pq = createPriorityQ(MAX_HEAP_SIZE);

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

    freePriorityQ(pq);
    pq = NULL;

    return huffmanTree;
}


void getHuffmanAlphabet(Node* node, int level, char currentCode[], char* huffmanAlphabet[]) {

    if ((node->left == NULL) && (node->right == NULL)) {
        currentCode[level] = 0;  // end of string
        huffmanAlphabet[node->value] = strdup(currentCode);
    }
    else {
        // left -> 0
        currentCode[level] = '0';
        getHuffmanAlphabet(node->left, level + 1, currentCode, huffmanAlphabet);

        // right -> 1
        currentCode[level] = '1';
        getHuffmanAlphabet(node->right, level + 1, currentCode, huffmanAlphabet);
    }
}

void printHuffmanAlphabet(char* huffmanAlphabet[]) {
    for (int i=0; i<MAX_HEAP_SIZE; i++){
        printf("%d\t%c\t%s\n", i, i, huffmanAlphabet[i]);
    }
}


void freeHuffmanAlphabet(char* huffmanAlphabet[]) {
    for (int i=0; i<MAX_HEAP_SIZE; i++){
        free(huffmanAlphabet[i]);
        huffmanAlphabet[i] = NULL;
    }
}


/*
** Write a single huffman encoded bit to the output file chunk buffer.
** Returns true in case of success, false otherwise.
*/
bool fwriteBitInBuffer(char bit, char buffer[], int* nbits, ull* nbytes) {
    buffer[*nbytes] <<= 1;
    if (bit == '1') buffer[*nbytes] |= 1;

    (*nbits)++;
    if (*nbits == 8) {
        *nbits = 0;
        (*nbytes)++;
    }

    if (*nbytes == MAX_ENCODED_BUFFER_SIZE) {
        return false;
    }

    return true;
}