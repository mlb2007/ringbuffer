CC=gcc
CPP=g++
CFLAGS=-g -Wall
LDFLAGS=

all: qbTest.o
	${CPP} ${LDFLAGS} -o qbTest -lpthread -lm $<

%.o: %.cpp Makefile
	${CPP} ${CFLAGS} -c $<

clean:
	rm -f *.d *.o qbTest
