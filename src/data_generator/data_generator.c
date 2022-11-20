#include <stdio.h>
#include <stdlib.h>

typedef unsigned long long ull;

// 1MB, 5MB, 10MB, 50MB, 100MB, 500MB, 1GB, [5GB, 10GB]
// media su tre run

// 1, 2, 4, 6, 8, 10, 12, 16, 20, 24

#define NUMFILES 9

char* output[NUMFILES] = {
    "/home/michele.yin/HuffmanCODEC/syntheticdata/input_1mb.bin",
    "/home/michele.yin/HuffmanCODEC/syntheticdata/input_5mb.bin",
    "/home/michele.yin/HuffmanCODEC/syntheticdata/input_10mb.bin",
    "/home/michele.yin/HuffmanCODEC/syntheticdata/input_50mb.bin",
    "/home/michele.yin/HuffmanCODEC/syntheticdata/input_100mb.bin",
    "/home/michele.yin/HuffmanCODEC/syntheticdata/input_500mb.bin",
    "/home/michele.yin/HuffmanCODEC/syntheticdata/input_1gb.bin",
    "/home/michele.yin/HuffmanCODEC/syntheticdata/input_5gb.bin",
    "/home/michele.yin/HuffmanCODEC/syntheticdata/input_10gb.bin"
};


ull filesizes[NUMFILES] = {
    (ull)1024*1024,
    (ull)5*1024*1024,
    (ull)10*1024*1024,
    (ull)50*1024*1024,
    (ull)100*1024*1024,
    (ull)500*1024*1024,
    (ull)1024*1024*1024,
    (ull)5*1024*1024*1024,
    (ull)10*1024*1024*1024
};


int generateFile (char* filename, ull size)
{
    printf("Working on %s with %llu bytes\n", filename, size);

	ull i, j;

	char* alphabet = "abcdefghijklmnopqrstuvwxyz ";
	int englishLetterPercent [27] = {8, 2, 3, 4, 12, 2, 2, 6, 7, 2, 0, 4, 2, 6, 7, 2, 1, 5, 5, 7, 3, 1, 2, 0, 2, 0, 5};
	int*   englishLetterFreq = (int *) malloc(sizeof (int) * 100);	

	FILE *f;

	f = fopen (filename, "w");
	if (f == NULL) {
		printf ("Error opening file!\n");
		exit(1);
	}

	/* fill the letter array */
	int   range = 0;
	int lastMax = 0;
	for (i = 0; i < 27; i++)
	{
		range += englishLetterPercent[i];
		for (j = lastMax; j < range; j++)
		{
			englishLetterFreq[j] = i;
		}

		lastMax = range + 1;
	}
	/* fill the file */
	for (i = 0; i < size; i++) {
		fprintf (f, "%c", alphabet[englishLetterFreq[rand() % 100]]);
	}
    free(englishLetterFreq);
	fclose(f);

	return 0;
}


int main(){
    int index = 0;
    for (index=0; index<NUMFILES; index++){
        generateFile(output[index], filesizes[index]);
    }
    return 0;
}