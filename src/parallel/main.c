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
#include <time.h>

#include <mpi.h>
#ifdef _OPENMP 
# include <omp.h> 
#endif

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


    // gather all files in the input directory and count the size of each file
    char files[files_count][PATH_MAX];
    ull file_sizes[files_count];
    char sorted_files[files_count][PATH_MAX];
    int sorted_file_indexes[files_count];
    int files_per_process[size];
    if(rank ==0 ){
        char* files_ptr[files_count];
        char* sorted_files_ptr[files_count];
        for (int i = 0;i < files_count;i++) {
            files_ptr[i] = files[i];
            sorted_files_ptr[i] = sorted_files[i];
        }
        int current = 0;
        listFiles(inputname, &current, (char**)files_ptr);
        printf("Files:\n");
        // for (int i = 0;i < files_count;i++) {
        //     printf("%s\n", files[i]);
        //     printf("strlen %d\n", strlen(files[i]));
        // }
        fileSizeCounter((char **)files_ptr, files_count,0,file_sizes);
        for (int i = 0;i < files_count;i++) {
            printf("%s\n", files[i]);
            printf("size %llu\n", file_sizes[i]);
        }
        fileSorterSize((char **)files_ptr, file_sizes, files_count,size,(char **)sorted_files_ptr,sorted_file_indexes,files_per_process);
        printf("Sorted Files:\n");
        int count =0 ;
        for (int i = 0;i < files_count;i++) {
            if(i==sorted_file_indexes[count]){
                printf("Process %d is assigned these files\n",count);
                count++;
            }
            printf("%s\n", sorted_files[i]);
        }
    }

    char process_input_files[files_count][PATH_MAX];
    char process_output_files[files_count][PATH_MAX];
    int process_count = 0;
     // Last process may get some more because of uneven integer division
    fileDistributerSize((char**)sorted_files, sorted_file_indexes,files_per_process, files_count, rank, size, (char**)process_input_files,&process_count);
    MPI_Barrier(MPI_COMM_WORLD);
    // for(int i=0;i<process_count;i++){
    //     printf("Process %d is assigned %s\n",rank,process_input_files[i]);
    // }
    
    // call encoder for each process
    for(int i=0;i<process_count;i++){
        printf("Rank %d is processing\n",rank);
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
        clock_t start_cpu = clock();
        double start_wall = MPI_Wtime();
        (*processing)((char *)process_input_files[i],(char *)process_output_files[i],num_threads);
        clock_t end_cpu = clock();
        double end_wall =  MPI_Wtime();
        double cpu_time_used = ((double)(end_cpu - start_cpu)) / CLOCKS_PER_SEC;
        double wall_time = (double) (end_wall-start_wall);
        printf("CPU Time required to process %f\n", cpu_time_used);
        printf("Wall Time required to process %f\n", wall_time);
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
        printf("saving as file %s\n",outputname);
        clock_t start_cpu = clock();
        double start_wall = MPI_Wtime();
        (*processing)((char *)inputname,(char *)outputname,num_threads);
        clock_t end_cpu = clock();
        double end_wall =  MPI_Wtime();
        double cpu_time_used = ((double)(end_cpu - start_cpu)) / CLOCKS_PER_SEC;
        double wall_time = (double) (end_wall-start_wall);
        printf("CPU Time required to process %f\n", cpu_time_used);
        printf("Wall Time required to process %f\n", wall_time);
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
    int num_threads = 1;
    #ifdef _OPENMP 
        num_threads = omp_get_max_threads();
    #endif
    // input processing
    char option = '0';
    while ((option = getopt(argc, argv, "edrh")) != -1) {
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
            case 'h':
                printf("-e to encode\n");
                printf("-d to decode\n");
                printf("-r to recursively process a whole directory\n");
                printf("Examples of usage\n");
                printf("%s -e input.txt output.txt\n",argv[0]);
                printf("%s -d input.txt output.txt\n",argv[0]);
                printf("%s -r -e /inputfolder /outputfolder\n",argv[0]);
                printf("%s -r -d /inputfolder /outputfolder\n",argv[0]);
                printf("%s -r -d /inputfolder /outputfolder\n",argv[0]);
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