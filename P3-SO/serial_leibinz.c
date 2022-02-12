#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#define CORES_NUMBER 2

long double calc_leibinz(unsigned long int iteration);

int main(int argc, char *args[]){
	unsigned long int iterations;
	long double leibinz_result = 0;
	
	unsigned long long start_tu;	// Variables de toma de tiempo us & s
	unsigned long long stop_tu;
	unsigned long long start_ts;
	unsigned long long stop_ts;
	struct timeval ts;
	float elapsed_time;

	if(argc < 2)
		iterations = 2000000000;
	else
		iterations = atoi(args[1]);

	printf("\nIteraciones a realizar: %li\n\n", iterations);

	gettimeofday(&ts, NULL);
	start_tu = ts.tv_usec; // Tiempo inicial
	start_ts = ts.tv_sec; // Tiempo inicial

	for(int idx=0; idx<iterations; idx++){	// Nuestro algoritmo se basa en que si es positivo se suman al resultado que tenemos y si es negativo o impar las iteranciones se restan
		if(idx%2==0 || idx==0)
			leibinz_result += calc_leibinz(idx);
		else
			leibinz_result -= calc_leibinz(idx);
	}

	gettimeofday(&ts, NULL);
	stop_tu = ts.tv_usec; // Tiempo final
	stop_ts = ts.tv_sec; // Tiempo final

	printf("\nEl resultado de la serie de Leibinz fue de: %.20LF\n\n", leibinz_result);
	printf("\nPi calculado con la serie de Leibinz es de: %.20LF\n\n", leibinz_result*4);
	printf("------------------------------\n");
	printf("TIEMPO TOTAL: %lld microsegundos\n",((stop_ts-start_ts)*1000000L+stop_tu) - start_tu);
	printf("TIEMPO TOTAL: %lf milisegundos\n", (float)(((stop_ts-start_ts)*1000000L+stop_tu) - start_tu)/1000.0);
	printf("TIEMPO TOTAL: %lf segundos\n", (float)(((stop_ts-start_ts)*1000000L+stop_tu) - start_tu)/1000000.0);

	return 0;
}

long double calc_leibinz(unsigned long int iteration){
	return (long double)1/((iteration*2)+1);
}
