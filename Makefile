.PHONY: all test clean

all: cq

libQueue.a: concurQueue.c
	gcc -fPIC -g -c concurQueue.c
	ar rcs libQueue.a concurQueue.o

cq: main.c libQueue.a
	gcc -g -Wall -pthread -o cq main.c libQueue.a

test: cq
	valgrind ./cq

clean: 
	rm -f cq libQueue.a vgcore.* concurQueue.o