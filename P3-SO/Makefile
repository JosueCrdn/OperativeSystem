main: clonado_leibinz.c posix_leibinz.c serial_leibinz.c paralelo_leibinz.c
	gcc -o clonado_leibinz clonado_leibinz.c
	gcc -pthread -o posix_leibinz posix_leibinz.c
	gcc -o serial_leibinz serial_leibinz.c
	gcc -o paralelo_leibinz paralelo_leibinz.c

.PHONY: clean
clean:
	rm clonado_leibinz
	rm posix_leibinz
	rm serial_leibinz
	rm paralelo_leibinz
