all: mensajes.out semaforos.out

mensajes.out:	mensajes.c
		gcc -o mensajes.out mensajes.c
semaforos.out:	semaforos.c
		gcc -o semaforos.out mensajes.c -lm
clean:
	rm *.out
