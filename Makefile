CC = gcc
flags = -std=c99 -g -O3 -funroll-loops -Wall -Wextra -Werror=pedantic -Werror=format=2 -pedantic-errors -Wno-unknown-pragmas
libs = -pthread -lgd

paralelo-simples: main_simples.o filehandler.o imagehandler.o help.o
	${CC} ${flags} -o simples/paralelo-simples main.o filehandler.o imagehandler.o help.o ${libs};
	make clean;

paralelo-complexo: main_complexo.o filehandler.o imagehandler.o help.o
	${CC} ${flags} -o complexo/paralelo-complexo main.o filehandler.o imagehandler.o help.o ${libs};
	make clean;

test-simples:
	rm -r -f Images/Small/Resize;\
	rm -r -f Images/Small/Thumbnail;\
	rm -r -f Images/Small/Watermark;\
	make paralelo-simples;

test-complexo:
	rm -r -f Images/Small/Resize;\
	rm -r -f Images/Small/Thumbnail;\
	rm -r -f Images/Small/Watermark;\
	make paralelo-complexo;

########################################################
main_simples.o: simples/main.c
	${CC} -c  ${flags} simples/main.c ${libs}

main_complexo.o: complexo/main.c
	${CC} -c  ${flags} complexo/main.c ${libs}

help.o: help.h help.c
	${CC} -c ${flags} help.c ${libs}

filehandler.o: filehandler.h filehandler.c
	${CC} -c ${flags} filehandler.c ${libs}

imagehandler.o: imagehandler.h imagehandler.c
	${CC} -c ${flags} imagehandler.c ${libs}

.PHONY : clean
clean:
	rm -f *.o
