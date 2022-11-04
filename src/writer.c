#include <stdio.h>
#include <stdlib.h>


int main(int argc, char *argv[]){
    char * filename = argv[1];
    FILE* ptr;
    ptr = fopen(filename, "wb");
    if (ptr == NULL) {
        printf("Error opening file\n");
        return 1;
    }
    for(int i = 0; i < 256; i++){
        fprintf(ptr, "%c", (char) i);
    }
    fclose(ptr);

    return 0;
}