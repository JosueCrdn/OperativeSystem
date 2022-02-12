#define _GNU_SOURCE     // Importante para poder usar clone
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <sched.h>
#include <malloc.h>
#include <sys/types.h>
#include <sys/wait.h>

#define CORES_NUMBER 2
#define FIBER_STACK 1024*64	// 64Kbytes tamaño de pila

void calc_pi_4(unsigned long int *values);
long double calc_leibinz(unsigned long int iteration);

long double partial_results[CORES_NUMBER];

int main(int argc, char *args[]){
	unsigned long int iterations;
	long double leibinz_result = 0;
	void *stack;
	pid_t pid;
	int status;
	
	unsigned long long start_tu;
	unsigned long long stop_tu;
	unsigned long long start_ts;
	unsigned long long stop_ts;
	struct timeval ts;

	stack = malloc(FIBER_STACK * CORES_NUMBER);	// Asignación de memoria por numero de núcleos
	
	if ( stack == 0 ){
		perror( "malloc: Falló en asignar stack." );
		exit( 1 );
	}
    
    unsigned long int args_1[CORES_NUMBER][3];

	if(argc < 2)
		iterations = 2000000000;
	else
		iterations = atoi(args[1]);

	printf("\nIteraciones a realizar: %li\n\n", iterations);

	gettimeofday(&ts, NULL);
	start_tu = ts.tv_usec; // Tiempo inicial
	start_ts = ts.tv_sec; // Tiempo inicial

	for(int idx=0; idx<CORES_NUMBER; idx++){
        args_1[idx][0] = idx;
        args_1[idx][1] = iterations;
        args_1[idx][2] = CORES_NUMBER;

        // Se hace la clonacion compartiendo lo siguientes parametros
        pid = clone(calc_pi_4, (char*) stack + FIBER_STACK*(idx+1), 
				SIGCHLD |CLONE_FS|CLONE_FILES|CLONE_SIGHAND|CLONE_VM, &args_1[idx][0]);
        // Registros archivos y memoria

	    if (pid == -1){
			perror( "clone: Falló el clonado del subproceso." );
			exit( 2 );
		}

		usleep(1000);	// X Mejorar
    }

    for(int idx=0; idx<CORES_NUMBER; idx++){
        pid=wait(&status);		// Se esperan los subprecesos creados
		
		if ( pid == -1 ){
			perror( "wait: Un subproceso falló." );
			exit( 3 );
		}
    }

    for(int idx=0; idx<CORES_NUMBER; idx++)		// Cada proceso asigna su parte del resultado
    	leibinz_result += partial_results[idx];

	gettimeofday(&ts, NULL);
	stop_tu = ts.tv_usec; // Tiempo final
	stop_ts = ts.tv_sec; // Tiempo final

	free(stack);	// Se libera memoria

	printf("\nEl resultado de la serie de Leibinz fue de: %.20LF\n\n", leibinz_result);
	printf("\nPi calculado con la serie de Leibinz es de: %.20LF\n\n", leibinz_result*4);
	printf("---------------------------------------------\n");
	printf("TIEMPO TOTAL: %lld microsegundos\n",((stop_ts-start_ts)*1000000L+stop_tu) - start_tu);
	printf("TIEMPO TOTAL: %lf milisegundos\n", (float)(((stop_ts-start_ts)*1000000L+stop_tu) - start_tu)/1000.0);
	printf("TIEMPO TOTAL: %lf segundos\n", (float)(((stop_ts-start_ts)*1000000L+stop_tu) - start_tu)/1000000.0);

	return 0;
}


void calc_pi_4(unsigned long int *values){
	int iter_start, iter_jump, mod;
	unsigned long int thread_id = *values;
	unsigned long int iter_number = *(values+1);
	unsigned long int cores = *(values+2);
	long double leibinz_result = 0;

	iter_jump = iter_number / cores;
	iter_start = iter_jump * thread_id;
	mod = iter_number%cores;

	if(thread_id+1 == cores)
		iter_jump += mod;

	for(int idx=iter_start; idx<(iter_start+iter_jump); idx++){
		if(idx%2==0 || idx==0)
			leibinz_result += calc_leibinz(idx);
		else
			leibinz_result -= calc_leibinz(idx);
	}

	partial_results[thread_id] = leibinz_result;
}

long double calc_leibinz(unsigned long int iteration){
	return (long double)1/((iteration*2)+1);
}