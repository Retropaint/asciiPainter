CC     = g++
CFLAGS = -std=c++11 -Wall

B       = build
TARGET  = ${B}/asciiPainter
SRC	    = main.cpp
AGNOS_A = ${B}/libagnos.a

OUT     = -o ${TARGET}
LIBS    = -lncurses -lagnos
LINKDIR = -L${B}

${TARGET}: ${AGNOS_A}
	${CC} ${CFLAGS} ${LINKDIR} ${LIBS} ${SRC} ${OUT}

${AGNOS_A}: ${B}
	${CC} ${CFLAGS} -c agnos/agnosUnix.cpp -o ${AGNOS_A}

${B}:
	mkdir ${B}

.PHONY: distclean
distclean:
	rm -rf build
