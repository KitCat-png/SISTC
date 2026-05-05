#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <float.h>
#include <time.h>
#include <sys/time.h>

#define NELEM 20000000
#define NITER 33 
#define NPROCS 30

void rectangular2polar(double *, double *, int);

// --- Implementação da mytime ---
double mytime() { //gets the current system time in seconds
    struct timeval tp;
    struct timezone tzp;
    gettimeofday(&tp, &tzp);
    return ( (double) tp.tv_sec + (double) tp.tv_usec * 1.e-6 );
}

// --- Implementação da shmalloc ---
void *shmalloc(size_t size) { //unlike malloc, memory allocated here is shared between the parent and children bc of nmap with MAP_SHARED
    return mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);
}

int main() {
    double t0;
    double *dados_in = (double *) shmalloc(sizeof(double)*NELEM);
    double *dados_out = (double *) shmalloc(sizeof(double)*NELEM);

    printf("1170701 - %s\n", __FILE__);
	
	if(dados_in == MAP_FAILED || dados_out == MAP_FAILED) { //error handling to ensure the OS gave us the requested memory
        perror("mmap");
        exit(1);
    }

    srand48(time(NULL));
    for(int i = 0; i < NELEM; ++i) //fills array with random numbers, only parent does this
        dados_in[i] = 2*(drand48()-0.5)*DBL_MAX;

    t0 = mytime(); //records start time

    for(int j=0; j < NITER; ++j) { //repeats entire calculation 33 times (basically the whole loop is a benchmark test)
        for(int p = 0; p < NPROCS; p++) { //spawns 30 processes to work at the same tim
            pid_t pid = fork();
            if(pid == 0) { //each child calculates its own chunk of the array
                int chunk = NELEM / NPROCS;
                int start = p * chunk;
                rectangular2polar(&dados_out[start], &dados_in[start], chunk);
                exit(0); 
            }
        }
        for(int p = 0; p < NPROCS; p++) { //parent waits for all children to finish their chunk
            wait(NULL);
        }
    }

    printf("Computation took %.1f s\n", mytime() - t0);
    return 0;
}
