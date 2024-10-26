CC = cc
INCLUDEDIR = /usr/include
LIBDIR = /usr/lib
CFLAGS = -std=c99 -Wall -Wextra -Wimplicit-fallthrough -fPIC -O2 -g

HEADER = ini.h
SRC = ini.c
OBJ = ${SRC:.c=.o}
LIB = libini.a
SOLIB = libini.so

all: ${LIB} ${SOLIB}

${OBJ}: ${SRC} ${HEADER}
	${CC} ${CFLAGS} -c $<

${LIB}: ${OBJ}
	ar -rcs $@ $^

${SOLIB}: ${OBJ}
	${CC} ${CFLAGS} -shared -o $@ $^

install: ${LIB} ${SOLIB}
	install ${HEADER} ${DESTDIR}${INCLUDEDIR}
	install ${LIB} ${DESTDIR}${LIBDIR}
	install ${SOLIB} ${DESTDIR}${LIBDIR}

clean:
	rm -f *.o ${LIB} ${SOLIB}

.PHONY: all install
