CC=g++
CFLAGS=-std=c++11 -Wall

asciiPainter: libagnos.a
	${CC} ${CFLAGS} -L./ -lncurses -lagnos main.cpp -o asciiPainter

libagnos.a:
	${CC} ${CFLAGS} -c agnos/agnosUnix.cpp -o libagnos.a
