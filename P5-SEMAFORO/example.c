#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <libsem.h>
							
// Macro que incluye el código de la instrucción máquina xchg 
#define atomic_xchg(A,B)  __asm__ __volatile__( \
  " lock xchg %1,%0 ;\n"  \
  : "=ir" (A)             \
  : "m" (B), "ir" (A)      \
);


#define CICLOS 10

char *pais[3]={"Peru","Bolivia","Colombia"};
int global=0;

SEMAPHORE exmut;

void *hilo1(void *arg)
{
	int *mynum=(int *) arg;
	int i=*mynum;

	int k;
	int l;

	for(k=0;k<CICLOS;k++)
	{	
		//convertir del ejemplo 1 al 2
		//l=1;
		//do { atomic_xchg(l,global); } while(l!=0); 
		waitsem(&exmut);
		
		// Inicia sección Crítica
		printf("Entra- %s",pais[i]);
		fflush(stdout);
		usleep(rand()%1000000);
		printf("- Sale %s \n",pais[i]);
		// Termina sección Crítica
		
		//global=0;
		//l=1;

		signalsem(&exmut);		
		// Espera aleatoria fuera de la sección crítica
		usleep(rand()%1000000);
	}
	exit(0); //salir del proceso
}

int main()
{
	pthread_t tid[3];
	int res;
	int args[3];
	int i;
	void *thread_result;

	srand(getpid());
	
	initsem(&exmut,1);
	
	// Crea los hilos
	for(i=0;i<3;i++)
	{
		args[i]=i;
		res = pthread_create(&tid[i], NULL, hilo1, (void *) &args[i]);
		//printf(res);
		//printf("tid %ld",*tid);

	}

	// Espera que terminen los hilos
	for(i=0;i<3;i++)
		res = pthread_join(tid[i], &thread_result);

}


