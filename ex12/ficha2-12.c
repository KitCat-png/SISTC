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
double mytime() {
    struct timeval tp;
    struct timezone tzp;
    gettimeofday(&tp, &tzp);
    return ( (double) tp.tv_sec + (double) tp.tv_usec * 1.e-6 );
}

// --- Implementação da shmalloc ---
void *shmalloc(size_t size) {
    return mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);
}

int main() {
    double t0;
    double *dados_in = (double *) shmalloc(sizeof(double)*NELEM);
    double *dados_out = (double *) shmalloc(sizeof(double)*NELEM);

    printf("1170701 - %s\n", __FILE__);
	
	if(dados_in == MAP_FAILED || dados_out == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }

    srand48(time(NULL));
    for(int i = 0; i < NELEM; ++i)
        dados_in[i] = 2*(drand48()-0.5)*DBL_MAX;

    t0 = mytime();

    for(int j=0; j < NITER; ++j) {
        for(int p = 0; p < NPROCS; p++) {
            pid_t pid = fork();
            if(pid == 0) {
                int chunk = NELEM / NPROCS;
                int start = p * chunk;
                rectangular2polar(&dados_out[start], &dados_in[start], chunk);
                exit(0); 
            }
        }
        for(int p = 0; p < NPROCS; p++) {
            wait(NULL);
        }
    }

    printf("Computation took %.1f s\n", mytime() - t0);
    return 0;
}
