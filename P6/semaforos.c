#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <math.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include "semaphoresarr.h"

//Definicion de varibales 
#define TAMBUFFER 10
#define VELPROD 10000	// Microsegundos
#define VELCONS 50000
#define PRODUCTORES 4
#define FIN -1

//Funciones 

void productor();
void consumidor();
int isprime(int n);

//Varibales globales 
int lim_inf,lim_sup,jumps;

struct STRBUFF {
	int ent;	// Donde va a estar el siguiente elemento que voy a meter al buffer
	int sal;	// Donde está el siguiente elemento que voy a sacar del buffer
	int buffer[TAMBUFFER];	// Buffer circular
};

struct TREE {
	int dato;
	struct TREE *left;
	struct TREE *right;
};
struct STRBUFF *bf;

void tree_insert(struct TREE **root,int dato);
void tree_inorder(struct TREE *root);
SEM_ID semarr;
enum {E_MAX,N_BLOK,S_EXMUT};  // Semáforos 0,1 y 2

int main(int argc, char *argv[])
{
    // Definición de variables
	int res;
	int n;
	int p;
	int shmid;
	int lista[4];	
	
	srand(getpid());
	
    // Creación del arreglo de semáforos
    semarr=createsemarray((key_t) 0x1234,3);

    initsem(semarr,E_MAX,TAMBUFFER);
    initsem(semarr,N_BLOK,0);
    initsem(semarr,S_EXMUT,1);

    printf("Semáforos creados\n");

	// Crear la memoria compartida
	shmid=shmget((key_t) 0x1235,sizeof(struct STRBUFF),0666|IPC_CREAT);
	bf=shmat(shmid,NULL,0);
	//Se crean los limites con los que trabjara cada proceso 
	lim_inf = atoi(argv[1]);
	lim_sup = atoi(argv[2]);
	int dif = lim_sup - lim_inf;
	jumps = dif / 4;

        /* Aquí se crean los procesos */
	for (int i = 0; i < PRODUCTORES; i++){
		//lista[i] = i;
		p=fork();
		if(p==0)
			productor(i);	//productor(lista[i]);
	}
	//Se crea 4 productores y un consumidor 
	p=fork();
	if(p==0)
		consumidor();

	//Aqui esperamos cada uno de los hijos creados
	for(n=0;n<2;n++)
		wait(NULL);
	/*
	semwait(semarr,E_MAX);	// Si se llena el buffer se bloquea
	semwait(semarr,S_EXMUT);	// Asegurar el buffer como sección crítica

	bf->buffer[bf->ent]=0;			
	bf->ent++;
	if(bf->ent==TAMBUFFER)	// Si TAMBUFFER es 10, 0 1 2 3 4 5 6 7 8 9
		bf->ent=0;		// 0 = Terminador     	
			
	semsignal(semarr,S_EXMUT);	// Libera la sección crítica del buffer
	semsignal(semarr,N_BLOK);	// Si el consumidor está bloqueado porque el buffer está vacío, lo desbloqueas
	
	wait(NULL);
    // Borrar los semáforos
	erasesem(semarr,E_MAX);
	erasesem(semarr,N_BLOK);
	erasesem(semarr,S_EXMUT);
	shmdt(bf);	// Desconectar la memoria compartida al terminar
	shmctl(shmid,IPC_RMID,NULL);	// Pedir al SO que elimine la memoria compartida
*/
	// Borrar los semáforos
        erasesem(semarr,E_MAX);
	erasesem(semarr,N_BLOK);
	erasesem(semarr,S_EXMUT);

	shmdt(bf);	// Desconectar la memoria compartida al terminar
	shmctl(shmid,IPC_RMID,NULL);	// Pedir al SO que elimine la memoria compartida



    exit(EXIT_SUCCESS);
}


void productor(int nprod)
{
    int n;	

    int inicio = lim_inf + (jumps * nprod);
    int termino = lim_inf + (jumps * (nprod+1));
    if (nprod ==3 )
	    termino= lim_sup+1;

    printf("Inicia productor\n");
    
    for(n = inicio;n <= termino; n++){
	    if(isprime(n) || n==termino){
		    semwait(semarr,E_MAX);	// Si se llena el buffer se bloquea
		    semwait(semarr,S_EXMUT);	// Asegurar el buffer como sección crítica
		    if(n<termino){
			
        		bf->buffer[bf->ent]=n;
			printf("Buscador %d encuentra %d\n",nprod,n);
		    }			
		    else {
			bf->buffer[bf->ent]=FIN;
		    }

			bf->ent++;
			if(bf->ent==TAMBUFFER)	// Si TAMBUFFER es 10, 0 1 2 3 4 5 6 7 8 9
				bf->ent=0;		// 0 = Terminador
		
        	usleep(rand()%VELPROD);
        	semsignal(semarr,S_EXMUT);	// Libera la sección crítica del buffer
        	semsignal(semarr,N_BLOK);	// Si el consumidor está bloqueado porque el buffer está vacío, lo desbloqueas

        	usleep(rand()%VELPROD);
		}
    }
    exit(0);
}


void consumidor()
{
	struct TREE *root=NULL;
	int productores=PRODUCTORES;
	int terminadores=0;
	int dato;

 //int n=1;

    printf("Inicia Consumidor\n");
    while(productores)
    {
        semwait(semarr,N_BLOK);	// Si el buffer está vacío, se bloquea
        semwait(semarr,S_EXMUT);	// Asegura el buffer como sección crítica 

		dato=bf->buffer[bf->sal];
		if(dato!=FIN)
		{
			// En vez de mostrarlo, lo vamos a guardar en un contenedor de manera
			// que queden ordenados en orden ascendente, por ejemplo, un árbol binario
        	// printf("\t\t\tConsumidor consume %d\n",dato);
			tree_insert(&root,dato);
		}
		else
		{
			productores--;
			terminadores++;
			printf("\t\t\tFINAL ENCONTRADO\n");
		}

		bf->sal++;
		if(bf->sal==TAMBUFFER)
			bf->sal=0;
        usleep(rand()%VELCONS);

        semsignal(semarr,S_EXMUT);	// Libera la SC el buffer
        semsignal(semarr,E_MAX);	// Si el productor está bloqueado porque el buffer estaba lleno, lo desbloquea
        usleep(rand()%VELCONS);

    }
	
	// Mostrar todos los elementos que se guardaron de manera ordenada en el contenedor.
printf("Lista ordenada de los numeros primos en el rango de %d a %d\n", lim_inf,lim_sup);
tree_inorder(root);	
printf("\tTerminadores %d\n",terminadores);    
exit(0);
}
     
int isprime(int n)
{
	int d=3;
	int prime=0;
	int limit=sqrt(n);
	
	if(n<2)
		prime=0;
	else if(n==2)
		prime=1;
	else if(n%2==0)
		prime=0;
	else
	{
		while(d<=limit && n%d)
			d+=2;
		prime=d>limit;
	}
	return(prime);
}
void tree_insert(struct TREE **root,int dato)
{
	if(*root==NULL)
	{
		*root=malloc(sizeof(struct TREE));
		(*root)->dato=dato;
		(*root)->left=NULL;
		(*root)->right=NULL;
	}
	else if(dato < (*root)->dato)
		tree_insert(&(*root)->left,dato);
	else if(dato > (*root)->dato)
		tree_insert(&(*root)->right,dato);

}

void tree_inorder(struct TREE *root)
{
	if(root->left!=NULL)
		tree_inorder(root->left);
	printf("%d\n",root->dato);
	if(root->right!=NULL)
		tree_inorder(root->right);
}
