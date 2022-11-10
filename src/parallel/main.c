//  example of compile mpicc -o counterMain ./fileCounter/counterMain.c ./datastructures/dictionary.c ./datastructures/priorityQ.c 
//  example of run mpiexec -n 2 ./counterMain ../data/ 


// every process needs to work on a list of chunks and they get redistributed to the processes

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <linux/limits.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>

#include <mpi.h>
#include <omp.h>

#include "../commons/commons.h"
#include "../datastructures/priorityQ.h"
#include "../datastructures/dictionary.h"
#include "encode.h"
#include "decode.h"
#include "huffman.h"
#include "frequency.h"
#include "folder.h"
#include "processDistributer.h"


/*
**  Function to process a directory recursively, splitting the files to a number of processes according to their raink
*/
int directoryProcesser(int rank,int size,char *inputname, char * outputname ,int num_threads,void (*processing)(char* arg1,char*arg2, int num_threads)){
    bool inputdirectory = false;
    int files_count = 0;
    if (rank == 0) {
        // check if input is a file or directory
        inputdirectory = regularDirectory((const char*)inputname) && directoryExists((const char*)inputname);
        if(inputdirectory){
            slashFolder((char*)inputname);
            slashFolder((char*)outputname);
            countFiles(inputname, &files_count);
        }else{
            printf("input %s is not a directory\n",inputname);
            MPI_Finalize();
            exit(1);
        }
        // check if output file folder already exists to prevent override
        bool outputdirectory = directoryExists((const char*)outputname);
        if (outputdirectory){
            printf("%s output already exists\n",outputname);
            MPI_Finalize();
            exit(1); 
        }
        printf("Number of files: %d\n", files_count);
    }

    // broadcast the files count, and if input is a single file or directory to all other nodes
    MPI_Bcast(&files_count, 1, MPI_INT, 0, MPI_COMM_WORLD);
    char files[files_count][PATH_MAX];
    char* files_ptr[files_count];
    for (int i = 0;i < files_count;i++) {
        files_ptr[i] = files[i];
    }
    // TO DO test for single file 
    // rank 0 also gathers all files in subfolder
    if (rank == 0) {
        int current = 0;
        listFiles(inputname, &current, (char**)files_ptr);
        printf("Files:\n");
        for (int i = 0;i < files_count;i++) {
            printf("%s\n", files[i]);
            printf("strlen %d\n", strlen(files[i]));
        }
    }
    char process_input_files[files_count][PATH_MAX];
    char process_output_files[files_count][PATH_MAX];
    int process_count = 0;
    MPI_Barrier(MPI_COMM_WORLD);

    //  files are distributed evenly across processes. 
    // Last process may get some more because of uneven integer division
    fileDistributer((char**)files, files_count, rank, size, (char**)process_input_files, &process_count);
    MPI_Barrier(MPI_COMM_WORLD);

    // call encoder for each process
    for(int i=0;i<process_count;i++){
        int position = 0;
        // create new names
        strcpy((char*)process_output_files[i],(char*)outputname);
        position = matchingCount((char*)process_input_files[i],(char*)inputname);
        strcat((char*)process_output_files[i],(char*)process_input_files[i]+position);
        //strcat((char*)process_output_files[i],".huf");
        // making all sub directories recursively
        char sav;
        bool changed = false;
        char *pos = strrchr(process_output_files[i], '/');
        if (pos != NULL) {
            sav = *pos;
            changed = true;
            *pos = '\0';
        }
        printf("making directory %s\n",process_output_files[i]);
        recursivemkdir(process_output_files[i]);
        printf("done with directory %s\n",process_output_files[i]);
        if(changed){
            *pos = sav;
            changed = false;
        }
        // processsing the file
        printf("processing file %s\n",process_input_files[i]);
        printf("saving as file %s\n",process_output_files[i]);
        (*processing)((char *)process_input_files[i],(char *)process_output_files[i],num_threads);
    }
    MPI_Barrier(MPI_COMM_WORLD);
    return 0;
}


/*
**  Function to process a file. Only rank 0 wil execute
*/

int fileProcesser(int rank,char *inputname, char * outputname,int num_threads, void (*processing)(char* arg1,char*arg2,int num_threads)){
     bool inputFile = false;
     int files_count = 1;
     if (rank == 0) {
        // check if input is a file and exists
        inputFile = regularFile((const char*)inputname) && fileExists((const char*)inputname);
        if(!inputFile){
            printf("input %s is not a file\n",inputname);
            MPI_Finalize();
            exit(1);
        }
        // check if output file already exists to prevent override
        bool outputfile = fileExists((const char*)outputname);
        if (outputfile){
            printf("%s output already exists\n",outputname);
            MPI_Finalize();
            exit(1); 
        }
        printf("Number of files: %d\n", files_count);
        // processsing the file
        printf("processing file %s\n",inputname);
        printf("saving as file %s\n",outputfile);
        (*processing)((char *)inputname,(char *)outputname,num_threads);
    }
}

int main(int argc, char** argv) {
    // initialize MPI
    int provided;
    MPI_Init_thread(&argc, &argv, MPI_THREAD_FUNNELED, &provided);
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // read and preprocess folders
    char inputname[PATH_MAX];
    char outputname[PATH_MAX];
    int files_count = 0;
    bool inputfile = true;
    void *processingFunction = NULL;
    int num_threads = 4;
    // input processing
    char option = '0';
    while ((option = getopt(argc, argv, "edrp:h")) != -1) {
        switch (option) {
            case 'e':
                processingFunction = fileEncoderFull;
                break;
            case 'd':
                processingFunction = fileDecoderFull;
                break;
            case 'r':
                inputfile = false;
                break;
            case 'p':
                num_threads = atoi(optarg);
                break;
            case 'h':
                printf("-e to encode\n");
                printf("-d to decode\n");
                printf("-r to recursively process a whole directory\n");
                printf("-p to set the number of threads for each process. Defaults to 4 otherwise\n");
                printf("Examples of usage\n");
                printf("%s -e input.txt output.txt\n",argv[0]);
                printf("%s -d input.txt output.txt\n",argv[0]);
                printf("%s -r -e /inputfolder /outputfolder\n",argv[0]);
                printf("%s -r -d /inputfolder /outputfolder\n",argv[0]);
                printf("%s -r -d -p 10 /inputfolder /outputfolder\n",argv[0]);
                MPI_Finalize();
                exit(1);
        }
    }
    if (optind+1<argc){
        strcpy(inputname,argv[optind]);
        strcpy(outputname,argv[optind+1]);
    }else{
        printf("Missing input and/or output files\n");
        MPI_Finalize();
        exit(1);
    }

    printf("Running on num_threads %d\n",num_threads);
    if (inputfile){
        fileProcesser(rank,inputname,outputname,num_threads,processingFunction);
    }else{
        directoryProcesser(rank,size,inputname,outputname,num_threads,processingFunction);
    }

    MPI_Finalize();
    return 0;
}