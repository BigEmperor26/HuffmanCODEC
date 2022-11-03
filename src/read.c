//  example of run "mpiexec -n 4 ./read test.bin"

#include <stdio.h>
#include <stdlib.h>
#include "priorityQ.h"
#include <mpi.h>


// the key is the char as integer
// size is the number of different character or bytes, so 1 byte = 256
// the character is the key as integer, the frequency is the value
typedef struct Dictionary {
    ull frequency[MAX_HEAP_SIZE];
    int size;
} Dictionary;


Dictionary *  createDictionary(int size){
    Dictionary * d = (Dictionary*)malloc(sizeof(Dictionary));
    for(int i = 0; i < size; i++){
        d->frequency[i] = 0;
    }
    d->size = size;
    return d;
}

void freeDictionary(Dictionary * d){
    free(d);
    return;
}

void printDictionary(Dictionary * d){
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

// with offset each partition is up to 4gb
int counter(MPI_File *readfile, Dictionary *d, ull offset, ull size){
    // with binary offset
    char c;
    ull count = 0;
    ull read = 0;
    MPI_File_seek(*readfile, offset, MPI_SEEK_SET);
    while ((MPI_File_read(*readfile, &c, 1, MPI_CHAR, MPI_STATUS_IGNORE) == MPI_SUCCESS) && (count<size)){
        // printf("rank %d read %c \n",rank, c);
        d->frequency[c]++;
        count++;
    }
    return 0;
}


int main(int argc, char ** argv) {
    MPI_Init(&argc, &argv);

    char * filename = argv[1];
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    int file_size = 0;
    int offset = 0;
    int size_to_read = 0;
    MPI_Barrier( MPI_COMM_WORLD );
    // if process 0, get the file size
    if(rank==0){
        printf("World size is %d\n", size);
        FILE *ptr;
        ptr = fopen(filename,"rb"); 
        if (ptr == NULL){
            printf("Error opening file\n");
            return 1;
        }
        fseek(ptr, 0, SEEK_END); // seek to end of file
        file_size = ftell(ptr); // get current file pointer
        fclose(ptr);
        printf("File size : %d\n", file_size);
    }
    MPI_Barrier( MPI_COMM_WORLD );
    // broadcast the file size to all processes
    MPI_Bcast(&file_size, 1, MPI_INT, 0, MPI_COMM_WORLD);
    // compute the offset for each process

    // if file size < number of processes, then only file size processes are needed
    int active_processes = size;
    if(file_size<size){
        active_processes = file_size;
    }
    if(rank<active_processes){
        offset = file_size/active_processes * rank;
        size_to_read = file_size/active_processes;
        if (rank == active_processes-1){
            size_to_read = file_size - offset;
        }
    }else{
        offset = 0;
        size_to_read = 0;
    }
    printf("rank= %d, offset= %d, amount to read = %d \n", rank, offset, size_to_read);
    // for all processes, 
    // open the file
    MPI_File readfile;
    int rc = MPI_File_open(MPI_COMM_WORLD, filename ,MPI_MODE_RDONLY,MPI_INFO_NULL, &readfile);
    if(rc){
        printf("Error opening file\n");
        MPI_Abort(MPI_COMM_WORLD, rc);
    }
    // create a dictionary
    Dictionary * d = createDictionary(256);
    // count the frequency of each character
    if (rank<active_processes){
        counter(&readfile, d, offset, size_to_read);
    }
    MPI_File_close(&readfile);

    Dictionary * counts = createDictionary(256);
    MPI_Reduce( (d->frequency), counts->frequency, 256, MPI_UNSIGNED_LONG_LONG, MPI_SUM, 0, MPI_COMM_WORLD);
    if(rank==0){
        printDictionary(counts);
    }
    freeDictionary(d);
    MPI_Finalize();

}