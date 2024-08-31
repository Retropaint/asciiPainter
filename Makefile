CC=g++
CFLAGS=-std=c++11 -Wall
B=build/

${B}asciiPainter: ${B}libagnos.a
	${CC} ${CFLAGS} -L${B} -lncurses -lagnos main.cpp -o ${B}asciiPainter

${B}libagnos.a: ${B}
	${CC} ${CFLAGS} -c agnos/agnosUnix.cpp -o ${B}libagnos.a

${B}:
	mkdir ${B}

.PHONY: distclean
distclean:
	rm -rf build
