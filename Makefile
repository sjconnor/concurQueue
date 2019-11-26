.PHONY: all test clean

all: main.c concurQueue.c
	gcc -fPIC -c concurQueue.c
	gcc -shared -o libconnorlib.so concurQueue.o
	gcc -o foo foo.o -L. -lmylib

test: main.c concurQueue.c
	gcc -g -Wall -pthread -o cq concurQueue.c main.c
	valgrind ./cq

clean: 
	$(RM) cq