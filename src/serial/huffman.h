#ifndef __HUFFMAN__
#define __HUFFMAN__

#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include "../commons/commons.h"
#include "../datastructures/dictionary.h"
#include "../datastructures/priorityQ.h"

ull get_frequencies(FILE* file, Dictionary* dict);

Node* getHuffmanTree(Dictionary* dict);

void getHuffmanAlphabet(Node* node, int level, char currentCode[], char* huffmanAlphabet[]);

void printHuffmanAlphabet(char* huffmanAlphabet[]);

void freeHuffmanAlphabet(char* huffmanAlphabet[]);

/*
** Write a single huffman encoded bit to the output file chunk buffer.
** Returns true in case of success, false otherwise.
*/
bool fwriteBitInBuffer(char bit, FILE* outputFile, char buffer[], int* nbits, ull* nbytes, ull* outputSize);

#endif
