#define _GNU_SOURCE // sched_getcpu(3) is glibc-specific (see the man page)

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
#include <sched.h>
#include <mpi.h>
#ifdef _OPENMP 
# include <omp.h> 
#endif

int main(int argc, char **argv){
     // initialize MPI
    int provided;
    MPI_Init_thread(&argc, &argv, MPI_THREAD_FUNNELED, &provided);
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size); 
    int threads = omp_get_max_threads();
    double rank_0 = 0;
    if(rank==0){
        printf("Processes for MPI %d\n",size);
        printf("Threading for OMP %d\n",threads);
        rank_0 = MPI_Wtime();
    }
    double process = omp_get_wtime();
    // #ifdef _OPENMP 
    // omp_set_dynamic(0);
    // omp_set_num_threads(threads); 
    // #endif
    #pragma omp parallel
    {   
        double time = omp_get_wtime();
        int thread_id = omp_get_thread_num();
        int cpu_num = sched_getcpu();
        sleep(1);
        double end = omp_get_wtime();
        printf("Thread %d of process %d run on CPU %d slept %f\n",thread_id,rank,cpu_num,end-time);
    }
    #pragma omp barrier
    double process_end = omp_get_wtime();
    printf("Process rank %d took %f\n",rank,process_end-process);
    MPI_Barrier(MPI_COMM_WORLD);
    double rank_0_end = 0;
    if(rank==0){
        rank_0_end = MPI_Wtime();
        printf("Overall Execution time %f\n",rank_0_end-rank_0);
    }
    MPI_Finalize();
    return 0;
}