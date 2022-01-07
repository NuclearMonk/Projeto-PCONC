CC = gcc
flags = -g -O3
libs = -pthread -lgd

all: ap-paralelo-simples clean

ap-paralelo-simples: main.o filehandler.o imagehandler.o help.o
	${CC} ${flags} -o ap-paralelo-simples main.o filehandler.o imagehandler.o help.o ${libs}

main.o: main.c
	${CC} -c  ${flags} main.c ${libs} 

help.o: help.h help.c
	${CC} -c ${flags} help.c ${libs}

filehandler.o: filehandler.h filehandler.c
	${CC} -c ${flags} filehandler.c ${libs} 

imagehandler.o: imagehandler.h imagehandler.c
	${CC} -c ${flags} imagehandler.c ${libs} 
.PHONY : clean
clean:
	rm -f *.o