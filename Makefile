CC = gcc
flags = -g -o
libs = -pthread -lgd
name = NukeMonk


serialSimples: serial-simples.c
	${CC} serial-simples.c ${flags} ${name} ${libs}

ap-paralelo-simples: clean
	${CC} ap-paralelo-simples.c ${flags} ap-paralelo-simples ${libs}

clean:
	-rm ap-paralelo-simples