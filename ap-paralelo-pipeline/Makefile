CC = gcc
flags = -std=c99 -g -O3 -funroll-loops -Wall -Wextra -Werror=pedantic -Werror=format=2 -pedantic-errors -Wno-unknown-pragmas
libs = -pthread -lgd

pipeline: main.o filehandler.o imagehandler.o help.o
	${CC} ${flags} -o ap-paralelo-pipeline main.o filehandler.o imagehandler.o help.o ${libs};
	make clean;

main.o: main.c
	${CC} -c  ${flags} main.c ${libs}

help.o: Utils/help.h Utils/help.c
	${CC} -c ${flags} Utils/help.c ${libs}

filehandler.o: Utils/filehandler.h Utils/filehandler.c
	${CC} -c ${flags} Utils/filehandler.c ${libs}

imagehandler.o: Utils/imagehandler.h Utils/imagehandler.c
	${CC} -c ${flags} Utils/imagehandler.c ${libs}

.PHONY : clean
clean:
	rm -f *.o
