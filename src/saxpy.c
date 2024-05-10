/**
 * @defgroup   SAXPY saxpy
 *
 * @brief      This file implements an iterative saxpy operation
 * 
 * @param[in] <-p> {vector size} 
 * @param[in] <-s> {seed}
 * @param[in] <-n> {number of threads to create} 
 * @param[in] <-i> {maximum iterations} 
 *
 * @author     Danny Munera
 * @date       2020
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/time.h>
#include <pthread.h>

typedef struct {
    double* X;
    double* Y;
    double a;
    int start;
    int end;
    double* Y_avgs;
    int iters;
    int p;
} ThreadArgs;

void* saxpy_thread(void* arg) {
    ThreadArgs* args = (ThreadArgs*)arg;
    for (int it = 0; it < args->iters; it++) {
        for (int i = args->start; i < args->end; i++) {
            args->Y[i] = args->Y[i] + args->a * args->X[i];
        }
    }
    return NULL;
}

int main(int argc, char* argv[]){
    // Variables to obtain command line parameters
    unsigned int seed = 1;
    int p = 10000000;
    int n_threads = 2;
    int max_iters = 1000;
    // Variables to perform SAXPY operation
    double* X;
    double a;
    double* Y;
    double* Y_avgs;
    int i, it;
    // Variables to get execution time
    struct timeval t_start, t_end;
    double exec_time;

    // Getting input values
    int opt;
    while((opt = getopt(argc, argv, ":p:s:n:i:")) != -1){  
        switch(opt){  
            case 'p':  
                printf("vector size: %s\n", optarg);
                p = strtol(optarg, NULL, 10);
                assert(p > 0 && p <= 2147483647);
                break;  
            case 's':  
                printf("seed: %s\n", optarg);
                seed = strtol(optarg, NULL, 10);
                break;
            case 'n':  
                printf("threads number: %s\n", optarg);
                n_threads = strtol(optarg, NULL, 10);
                break;  
            case 'i':  
                printf("max. iterations: %s\n", optarg);
                max_iters = strtol(optarg, NULL, 10);
                break;  
            case ':':  
                printf("option -%c needs a value\n", optopt);  
                break;  
            case '?':  
                fprintf(stderr, "Usage: %s [-p <vector size>] [-s <seed>] [-n <threads number>] [-i <maximum iterations>]\n", argv[0]);
                exit(EXIT_FAILURE);
        }  
    }  
    srand(seed);

    printf("p = %d, seed = %d, n_threads = %d, max_iters = %d\n", p, seed, n_threads, max_iters);    

    X = (double*) malloc(sizeof(double) * p);
    Y = (double*) malloc(sizeof(double) * p);
    Y_avgs = (double*) malloc(sizeof(double) * max_iters);

    for(i = 0; i < p; i++){
        X[i] = (double)rand() / RAND_MAX;
        Y[i] = (double)rand() / RAND_MAX;
    }
    a = (double)rand() / RAND_MAX;

#ifdef DEBUG
    printf("Initial vector X= [ ");
    for(i = 0; i < 5; i++){  // print first 5 elements for brevity
        printf("%f, ",X[i]);
    }
    printf("... ]\n");

    printf("Initial vector Y= [ ");
    for(i = 0; i < 5; i++){  // print first 5 elements for brevity
        printf("%f, ", Y[i]);
    }
    printf("... ]\n");

    printf("Scalar a= %f \n", a);    
#endif

    pthread_t* threads = malloc(n_threads * sizeof(pthread_t));
    ThreadArgs* args = malloc(n_threads * sizeof(ThreadArgs));
    int segment_size = p / n_threads;

	gettimeofday(&t_start, NULL);
	for (it = 0; it < max_iters; it++) {
		for (i = 0; i < n_threads; i++) {
			args[i].X = X;
			args[i].Y = Y;
			args[i].a = a;
			args[i].start = i * segment_size;
			args[i].end = (i == n_threads - 1) ? p : (i + 1) * segment_size;
			args[i].iters = 1; // Each thread computes one iteration at a time
			pthread_create(&threads[i], NULL, saxpy_thread, &args[i]);
		}
		for (i = 0; i < n_threads; i++) {
			pthread_join(threads[i], NULL);
		}

		// Calculate average for this iteration
		double total = 0;
		for (i = 0; i < p; i++) {
			total += Y[i];
		}
		Y_avgs[it] = total / p;
	}
	gettimeofday(&t_end, NULL);

#ifdef DEBUG
    printf("Resultant vector Y= [ ");
    for(i = 0; i < 5; i++){  // print first 5 elements for brevity
        printf("%f, ", Y[i]);
    }
    printf("... ]\n");
#endif
    
    exec_time = (t_end.tv_sec - t_start.tv_sec) * 1000.0;  // sec to ms
    exec_time += (t_end.tv_usec - t_start.tv_usec) / 1000.0; // us to ms
    printf("Execution time: %f ms \n", exec_time);
    printf("Last 3 values of Y: %f, %f, %f \n", Y[p-3], Y[p-2], Y[p-1]);
    printf("Last 3 values of Y_avgs: %f, %f, %f \n", Y_avgs[max_iters-3], Y_avgs[max_iters-2], Y_avgs[max_iters-1]);

    free(X);
    free(Y);
    free(Y_avgs);
    free(threads);
    free(args);

    return 0;
}
