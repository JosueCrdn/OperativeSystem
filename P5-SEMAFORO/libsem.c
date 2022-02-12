#include <pthread_utils.h>
#include <libsem.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <signal.h>

// Macro que incluye el código de la instrucción máquina xchg
#define atomic_xchg(A,B)  __asm__ __volatile__( \
  " lock xchg %1,%0 ;\n"  \
  : "=ir" (A)             \
  : "m" (B), "ir" (A)      \
);

//variables globales en el semaforo
int g=0,h=0;

void initsem(SEMAPHORE *s,int val)
{
	int l=1;
	//al iniciar el semaforo cambiamos a 1 para bloquear al resto 
	do {atomic_xchg(l,g);} while(l!=0);
	//inicializar el valor del semaforo s con el valor de val
	s-> count = val;
	//inicializar la cola del semaforo que este vacia
	initqueue(&s-> queue);
	//desbloqueamos el semaforo
	g=0;
	l=1;
	//printf("salgo de iniciar semafor con valores %d y %d\n",g,l);
}


void waitsem(SEMAPHORE *s)
{
	int l=1;
	//printf("dentro de wait con valor de %d y %d\n",g,l);
	do { atomic_xchg(l,g); } while(l!=0);
	// Decrememntar el contador el semáforo s->count--
	//if (s->count > 0)	//restar si hay cuenta
	//	{
		s->count--;
	//}
	// Si el contador del semáforo es negativo
	//
	//else
	if (s->count < 0){
	// Poner el TID del hilo en la cola del semáforo s (s->queue)
		s->count--;
		pthread_t tid =pthread_self();
		enqueue(&s->queue,tid);
		g=0;
		l=1;
		block_thread();	//bloquear hilo que se obtenga con tid
//		printf("salgo de wait de bloquear con valor de %d y %d\n",g,l);
	}
	g=0;
	l=1;	//regresar los valores default
	//printf("salgo de wait sin bloquear con valor de %d y %d\n",g,l);
}

void signalsem(SEMAPHORE *s)
{
	//	printf("sennal a semaforo\n");
	int l =1;
	do { atomic_xchg(l,h); } while(l!=0);
	// Incrementar el contador del semáforo
	//if(s->count > 0)	//si no hay hilos esparando en la cola se suma
	//{
		s->count++;
	//}
	// Si el contador es <= 0 aún hilos bloqueados
	if (s->count <= 0)
	{
		s->count++;
		pthread_t get_tid = dequeue(&s->queue);// Sacar el TID del hilo de la cola de bloqueados del semáforo s (s->queue)
		unblock_thread(get_tid);// Desbloquear ese hilo usando su TID
	}
	l=1;
	h=0;	// regresar valores 
	//printf("salgo de signal con valor de %d y %d\n",h,l);
}
