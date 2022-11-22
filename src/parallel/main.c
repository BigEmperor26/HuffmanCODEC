//  example of compile mpicc -o counterMain ./fileCounter/counterMain.c ./datastructures/dictionary.c ./datastructures/priorityQ.c 
//  example of run mpiexec -n 2 ./counterMain ../data/ 


// every process needs to work on a list of chunks and they get redistributed to the processes

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
// #include <linux/limits.h>
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
int directoryProcesser(int rank,int size,char *inputname, char * outputname ,int num_threads,bool (*processing)(char* arg1,char*arg2, int num_threads,int mode,int rank),int mode){
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
        // // check if output file folder already exists to prevent override
        // bool outputdirectory = directoryExists((const char*)outputname);
        // if (outputdirectory){
        //     printf("%s output already exists\n",outputname);
        //     MPI_Finalize();
        //     exit(1); 
        // }
        printf("Number of files: %d\n", files_count);
    }
    //printf("Rank %d Number of files: %d\n",rank, files_count);
    MPI_Barrier(MPI_COMM_WORLD);
    // broadcast the files count, and if input is a single file or directory to all other nodes
    MPI_Bcast(&files_count, 1, MPI_INT, 0, MPI_COMM_WORLD);
    //printf("Rank %d Number of files: %d\n", rank, files_count);
    MPI_Barrier(MPI_COMM_WORLD);
    // gather all files in the input directory and count the size of each file
    char* files =(char*) malloc(sizeof(char)*files_count*PATH_MAX);
    ull * file_sizes = (ull*)malloc(sizeof(ull)*files_count);
    char* sorted_files = (char*)malloc(sizeof(char)*files_count*PATH_MAX);
    int * sorted_file_indexes = (int*)malloc(sizeof(int)*files_count);
    int * files_per_process =(int*) malloc(sizeof(int)*size);
    ull* files_sizes_per_process = (ull*)malloc(sizeof(ull)*size);

    ull total_size_to_process = 0;
    if(rank ==0 ){
        char ** files_ptr =(char**) malloc(sizeof(char*)*files_count);
        char** sorted_files_ptr =(char**) malloc(sizeof(char*)*files_count);
        for (int i = 0;i < files_count;i++) {
            files_ptr[i] = files+i*PATH_MAX;
            sorted_files_ptr[i] = sorted_files+i*PATH_MAX;
        }
        int current = 0;
        listFiles(inputname, &current, (char**)files_ptr);
        printf("Files:\n");
        fileSizeCounter((char **)files_ptr, files_count,0,file_sizes);
        for (int i = 0;i < files_count;i++) {
            printf("%s\n", files_ptr[i]);
        }
        // ull * pre_sorted_file_sizes = (ull*)malloc(sizeof(ull)*files_count);
        // char* pre_sorted_files = (char*)malloc(sizeof(char)*files_count*PATH_MAX);
        // char ** pre_sorted_files_ptr = (char**)malloc(sizeof(char*)*files_count);
        // for (int i = 0;i < files_count;i++) {
        //     pre_sorted_files_ptr[i] = pre_sorted_files+i*PATH_MAX;
        // }
        //filePreSorter((char **)files_ptr, file_sizes,files_count,pre_sorted_files_ptr,pre_sorted_file_sizes);
        // for (int i = 0;i < files_count;i++) {
        //      printf("%s  size %llu\n", pre_sorted_files_ptr[i],pre_sorted_file_sizes[i]);
        // }
        //fileSorterSize((char **)pre_sorted_files_ptr, pre_sorted_file_sizes, files_count,size,(char **)sorted_files_ptr,sorted_file_indexes,files_per_process,files_sizes_per_process);
        fileSorterSize((char **)files_ptr,file_sizes, files_count,size,(char **)sorted_files_ptr,sorted_file_indexes,files_per_process,files_sizes_per_process);
        ull total = 0;
        for(int i=0;i<files_count;i++){
            total += file_sizes[i];
        }
        printf("Total size of all files is %llu MiB\n",total/(1024*1024));
        free(files_ptr);
        free(sorted_files_ptr);
        // free(pre_sorted_file_sizes);
        // free(pre_sorted_files);
        // free(pre_sorted_files_ptr);

    }
    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Scatter(files_sizes_per_process, 1, MPI_UNSIGNED_LONG_LONG, &total_size_to_process, 1, MPI_UNSIGNED_LONG_LONG, 0, MPI_COMM_WORLD);
    MPI_Barrier(MPI_COMM_WORLD);
    char* process_input_files = (char*)malloc(sizeof(char)*files_count*PATH_MAX);//char process_input_files[files_count][PATH_MAX];
    char* process_output_files = (char*)malloc(sizeof(char)*files_count*PATH_MAX); //char process_output_files[files_count][PATH_MAX];
    int process_count = 0;
     // Last process may get some more because of uneven integer division
    MPI_Barrier(MPI_COMM_WORLD);
    fileDistributerSize((char*)sorted_files, sorted_file_indexes,files_per_process, files_count, rank, size, (char*)process_input_files,&process_count);
    MPI_Barrier(MPI_COMM_WORLD);
    printf("Process %d is assigned a total of %d files for a size of %llu MiB\n",rank,process_count,(ull)total_size_to_process/(1024*1024));
    //printf("Process %d is assigned a total of %d files for a size of %llu MiB\n",rank,process_count,(ull)total_size_to_process);
  
    //call encoder for each process
    for(int i=0;i<process_count;i++){
        printf("%d Rank %d is processing\n",rank,rank);
        int position = 0;
        // create new names
        strcpy(process_output_files+i*PATH_MAX,outputname);
        position = matchingCount(process_input_files+i*PATH_MAX,inputname);
        strcat(process_output_files+i*PATH_MAX,process_input_files+i*PATH_MAX+position);
        // making all sub directories recursively
        char sav;
        bool changed = false;
        char *pos = strrchr(process_output_files+i*PATH_MAX, '/');
        if (pos != NULL) {
            sav = *pos;
            changed = true;
            *pos = '\0';
        }
        recursivemkdir(process_output_files+i*PATH_MAX);
        if(changed){
            *pos = sav;
            changed = false;
        }
        // processsing the file
        printf("%d processing file %s\n",rank,process_input_files+i*PATH_MAX);
        printf("%d saving as file %s\n",rank,process_output_files+i*PATH_MAX);
        clock_t start_cpu = clock();
        double start_wall = MPI_Wtime();
        // bool completed = true;
        bool completed = (*processing)(process_input_files+i*PATH_MAX,process_output_files+i*PATH_MAX,num_threads,mode,rank);
        if(!completed){
            printf("%d Error on file %s\n",rank,process_output_files+i*PATH_MAX);
            exit(1);
        }
        clock_t end_cpu = clock();
        double end_wall =  MPI_Wtime();
        double cpu_time_used = ((double)(end_cpu - start_cpu)) / CLOCKS_PER_SEC;
        double wall_time = (double) (end_wall-start_wall);
        printf("%d CPU Time required to process %f\n", rank,cpu_time_used);
        printf("%d Wall Time required to process %f\n", rank,wall_time);
    }
    free(files);
    free(file_sizes);
    free(sorted_files);
    free(sorted_file_indexes);
    free(files_per_process);
    free(process_input_files);
    free(process_output_files);
    free(files_sizes_per_process);
    return 0;
}



/*
**  Function to process a file. Only rank 0 wil execute
*/

int fileProcesser(int rank,char *inputname, char * outputname,int num_threads, bool (*processing)(char* arg1,char*arg2,int num_threads,int mode,int rank),int mode){
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
        // // check if output file already exists to prevent override
        // bool outputfile = fileExists((const char*)outputname);
        // if (outputfile){
        //     printf("%s output already exists\n",outputname);
        //     MPI_Finalize();
        //     exit(1); 
        // }
        printf("Number of files: %d\n", files_count);
        // processsing the file
        printf("processing file %s\n",inputname);
        printf("saving as file %s\n",outputname);
        clock_t start_cpu = clock();
        double start_wall = MPI_Wtime();
        bool  completed = (*processing)((char *)inputname,(char *)outputname,num_threads,mode,rank);
        if(!completed){
            printf("Error on file %s\n",outputname);
            exit(1);
        }
        clock_t end_cpu = clock();
        double end_wall =  MPI_Wtime();
        double cpu_time_used = ((double)(end_cpu - start_cpu)) / CLOCKS_PER_SEC;
        double wall_time = (double) (end_wall-start_wall);
        printf("CPU Time required to process %f\n", cpu_time_used);
        printf("Wall Time required to process %f\n", wall_time);
    }
    return 0;
}

int main(int argc, char** argv) {
    // initialize MPI
    int provided;
    MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);
    //MPI_Init(&argc, &argv);
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    clock_t start_cpu = clock();
    double start_wall = MPI_Wtime();
    if(rank==0){
        printf("Running on num processes %d\n",size);  
    }
    // read and preprocess folders
    char inputname[PATH_MAX];
    char outputname[PATH_MAX];
    bool inputfile = true;
    void *processingFunction = NULL;
    int num_threads = 1;
    #ifdef _OPENMP 
        num_threads = omp_get_max_threads();
    #endif
    int mode = 0; // 0 for barrier, 1 for locks
    // input processing
    char option = '0';
    while ((option = getopt(argc, argv, "edrhbl")) != -1) {
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
            case 'b':
                mode = 0;
                break;
            case 'l':
                mode = 1;
                break;
            case 'h':
                printf("-e to encode\n");
                printf("-d to decode\n");
                printf("-r to recursively process a whole directory\n");
                printf("-b to use barrier parallelism\n");
                printf("-l to use locks parallelism\n");
                printf("Examples of usage\n");
                printf("%s -e input.txt output.txt\n",argv[0]);
                printf("%s -d input.txt output.txt\n",argv[0]);
                printf("%s -r -e /inputfolder /outputfolder\n",argv[0]);
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
    if (rank==0){
        printf("Running on num threads %d\n",num_threads); 
    }
    #ifndef _OPENMP 
        if(mode==1){
            printf("OpenMP not detected or supported. Locks mode requires OpenMP\n");
            MPI_Finalize();
            exit(1);
        }
    #endif
    if (inputfile){
        fileProcesser(rank,inputname,outputname,num_threads,processingFunction,mode);
    }else{
        directoryProcesser(rank,size,inputname,outputname,num_threads,processingFunction,mode);
    }
    clock_t end_cpu = clock();
    double end_wall =  MPI_Wtime();
    double cpu_time_used = ((double)(end_cpu - start_cpu)) / CLOCKS_PER_SEC;
    double wall_time = (double) (end_wall-start_wall);
    printf("%d Overall program CPU Time required to process the input for rank %d %s %f\n",rank,rank,inputname, cpu_time_used);
    printf("%d Overall program Wall Time required to process the inputfor rank %d %s %f\n",rank,rank,inputname, wall_time);
    MPI_Finalize();
    return 0;
}