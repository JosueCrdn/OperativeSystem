#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

struct msgbuf {
    long mtype;       
    char mtext[30];   
};

struct TREE {
	int dato;
	struct TREE *left;
	struct TREE *right;
};

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
		if(root->dato != 0){
			printf("%d\n",root->dato);
		}
	if(root->right!=NULL)
		tree_inorder(root->right);
}

int queue;	

void emisor(int num1, int num2)
{
	int i, j, flag;
	struct msgbuf mensaje;

	for (i = num1 + 1; i <= num2; ++i){
		flag = 0;
      	for (j = 2; j <= i/2; ++j){
        	if (i % j == 0)
         	{
            	flag = 1;
            	break;
         	}
    	}

      	if (flag == 0) {
      		printf("PRODUCIENDO ---> %d\n", i);

			mensaje.mtype=1;
			sprintf(mensaje.mtext,"%d",i);

			msgsnd(queue,&mensaje,sizeof(struct msgbuf),IPC_NOWAIT);	
			sleep(1);   
   		}
   	}
   	sleep(10);
   	sprintf(mensaje.mtext,"FIN");	
	msgsnd(queue,&mensaje,sizeof(struct msgbuf),IPC_NOWAIT);
	exit(0);    
}

void receptor()
{
	struct TREE *root=NULL;
	int dato;

	struct msgbuf mensaje;
	do 
	{
		msgrcv(queue,&mensaje,sizeof(struct msgbuf),1,0);
		dato = atoi(mensaje.mtext);
		tree_insert(&root,dato);
		//printf("%s\n",mensaje.mtext);
	} 
	while(strcmp(mensaje.mtext,"FIN")!=0); 
	printf("DATOS ORDENADOS\n");
	tree_inorder(root);	
	exit(0);
}

int main()
{
	int pid;
	int status;
	int rango = 0;
	
	printf("Enter the range: ");
	scanf("%d", &rango);
	int proceso1, proceso2;

	proceso1 = rango / 3;
	proceso2 = rango / 3;

	// Crear buzón de mensajes 
	queue=msgget(0x1234,0666|IPC_CREAT);
	if(queue==-1)
	{
		fprintf(stderr,"No se pudo crear el buzón\n");
		exit(1);
	}

	pid=fork();
	if(pid==0)
		emisor(1, proceso1);	

	pid=fork();
	if(pid==0)
		emisor(proceso1, proceso1 + proceso2);	

	pid=fork();
	if(pid==0)
		emisor(proceso1 + proceso2, rango);	

	pid=fork();
	if(pid==0)
		receptor();	
	
	wait(&status);
	wait(&status);
	wait(&status);
	wait(&status);
	
	msgctl(queue, IPC_RMID, NULL);
}
