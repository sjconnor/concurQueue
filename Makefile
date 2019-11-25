.PHONY: all test clean

all: main.c concurQueue.c 
	gcc -g -Wall -pthread -o cq concurQueue.c main.c

test: all
	valgrind ./cq

clean: 
	$(RM) cq