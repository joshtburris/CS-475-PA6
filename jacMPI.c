/*
 * jacobi.c
 * WimBo
 */

#include <stdio.h>
#include <stdlib.h>
#include "timer.h"
#include "mpi.h"

int main(int argc, char **argv) {

    int i, j, n, t0, t1, k, id, m, p, v, vp;
    double *prev, *cur, *temp, *guac, time;
    
    MPI_Status status;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);
    MPI_Comm_size(MPI_COMM_WORLD, &p);
    MPI_Barrier(MPI_COMM_WORLD);
    
    t0 = 0;
    t1 = 1;
    k = 2;
    m = 2000;
    v = 0;
    vp = 0;
    
    // Check command line args.
    if (argc > 1) {
        n = atoi(argv[1]);
    } else {
        printf("Usage : %s [N]\n", argv[0]);
        fflush(stdout);
        exit(1);
    }
    
    if (argc > 2) {
        m = atoi(argv[2]);
    }
    
    if (argc > 3) {
        k = atoi(argv[3]);
    }
    
    if (argc > 4) {
        v = 1;
        vp = atoi(argv[4]);
    }
   
    if (k < 2) k = 2;
    if (k > 500) k = 500;
    
    // Memory allocation for data array.
    prev = (double *) malloc( sizeof(double) * ((n/p) + (2*k)));
    cur = (double *) malloc( sizeof(double) * ((n/p) + (2*k)));
    if (prev == NULL || cur == NULL) {
        printf("[ERROR] : Fail to allocate memory.\n");
        fflush(stdout);
        exit(1);
    }
    if (id == 0) {
        guac = (double *)malloc(sizeof(double) * n);
    }
    
    
    // Initialization
    for (i = 0; i < (n/p) + (2*k); ++i) {
        prev[i] = i + id*(n/p) - k;
    }
   
    if (id == 0) {
        guac[0] = 0;
        guac[n-1] = n-1;
        cur[0] = 0;
        cur[(n/p) + (2*k) - 1] = (n/p) + (2*k) - 1;
    } else {
        cur[0] = id*(n/p) - k;
        cur[(n/p) + (2*k) - 1] = id*(n/p) + (2*k) - 1;
    }
    
    MPI_Barrier(MPI_COMM_WORLD);
    
    initialize_timer();
    start_timer();

    // Computation
    while (t0 < m) {
        
        for (i = t1; i < (n/p) + (2*k) - t1; ++i) {
            cur[i] = (prev[i-1] + prev[i] + prev[i+1]) / 3;
        }
        
        temp = prev;
        prev = cur;
        cur  = temp;
        ++t0;
        ++t1;
        
        if (t0 % k == 0 && p > 1) {
            t1 = 1;
            if (id == 0) {
                MPI_Send(&prev[n/p], k, MPI_DOUBLE, 1, 0, MPI_COMM_WORLD);
                MPI_Recv(&prev[(n/p)+k], k, MPI_DOUBLE, 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            } else if (id == p-1) {
                MPI_Send(&prev[k], k, MPI_DOUBLE, p-2, 0, MPI_COMM_WORLD);
                MPI_Recv(&prev[0], k, MPI_DOUBLE, p-2, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            } else {
                MPI_Send(&prev[k], k, MPI_DOUBLE, id-1, 0, MPI_COMM_WORLD);
                MPI_Recv(&prev[0], k, MPI_DOUBLE, id-1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                MPI_Send(&prev[n/p], k, MPI_DOUBLE, id+1, 0, MPI_COMM_WORLD);
                MPI_Recv(&prev[(n/p)+k], k, MPI_DOUBLE, id+1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }
       }
    }
    
    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Gather(&prev[k], n/p, MPI_DOUBLE, &guac[0], n/p, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    MPI_Barrier(MPI_COMM_WORLD);
    
    stop_timer();
    time = elapsed_time();
    
    if (id == 0) {
        if (v > 0) {
            for (i = 0; i < n; ++i) {
                printf("%f ", guac[i]);
                fflush(stdout);
            }
            printf("\n");
            fflush(stdout);
        }
        else {
            printf("first, mid, last: %f %f %f\n", guac[0], guac[n/2-1], guac[n-1]);
            fflush(stdout);
        }
        
        printf("Data size : %d  , #iterations : %d , time : %lf sec\n", n, t0, time);
        fflush(stdout);
    }
   
    free(prev);
    free(cur);
    //free(temp);
    //if (id == 0)
    //    free(guac);
    MPI_Finalize();
    return 0;
}



