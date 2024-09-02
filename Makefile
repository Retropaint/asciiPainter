CC     = g++
CFLAGS = -std=c++11 -Wall
B      = build

# static libs
ARC    = ${B}/libagnos.a
ARCSRC = agnos/agnosUnix.cpp

# compile target
${B}/asciiPainter: ${ARC} ${B}
	${CC} ${CFLAGS} main.cpp -L${B} -lncurses -lagnos -o $@

# compile static libs
${ARC}: ${ARCSRC} ${B}
	${CC} ${CFLAGS} -c $< -o $@

${B}:
	mkdir ${B}

.PHONY: clean
clean:
	rm -rf ${ARC}

.PHONY: distclean
distclean:
	rm -rf ${B}
