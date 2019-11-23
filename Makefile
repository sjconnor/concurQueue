all: main.c concurQueue.c 
	gcc -g -Wall -o cq concurQueue.c main.c

clean: 
	$(RM) cq