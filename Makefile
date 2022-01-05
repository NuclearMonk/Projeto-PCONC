CC = gcc
flags = -g -o
libs = -pthread -lgd




ap-paralelo-simples: clean
	${CC} ap-paralelo-simples.c ${flags} ap-paralelo-simples ${libs}

serial-simples: serial-simples.c
	${CC} serial-simples.c ${flags} serial-simples ${libs}


serial-complexo: serial-complexo.c
	${CC} serial-complexo.c ${flags} serial-complexo ${libs}

all: ap-paralelo-simples serial-simples serial-complexo

.PHONY : clean
clean:
	rm -f *.o ap-paralelo-simples serial-simples serial-complexo