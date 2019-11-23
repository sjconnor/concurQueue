#include "concurQueue.h"    // TODO needs to be a library
#include <stdio.h>
#include <stdlib.h>

// Returns an integer greater than, equal to, or less than 0, according as 
    //     the data identified by userArg is greater than, equal to, or less than 
    //     the data identified by ptr.
    //     ptr is a pointer to an element in the list. userArg is a ptr to any
    //     user-managed data (that may even be of different type)

// TODO remove this dummy
int matchFn(void *userArg, void *ptr) {

    return *(int*)userArg - *(int*)ptr; 

}


int main() {

    printf("You can do it!\n");

    // for testing
    int (*Queue_matchFn)(void*, void*) = &matchFn;

    // build a queue
    struct Queue *myQ = Queue_new(5);

    // some dumb data for now
    int a, b, c, d, e;
    a = 1;
    b = 2;
    c = 3;
    d = 4;
    e = 5;
    void *ptr1 = &a;
    void *ptr2 = &b;
    void *ptr3 = &c;
    void *ptr4 = &d;
    void *ptr5 = &e;

    Queue_add(myQ, ptr1);
    Queue_add(myQ, ptr2);
    Queue_add(myQ, ptr3);
    Queue_add(myQ, ptr4);
    Queue_add(myQ, ptr5);
    //Queue_add(myQ, ptr5); // this should error
    //Queue_add(myQ, ptr5);
    //Queue_add(myQ, ptr5);

    Queue_find(myQ, Queue_matchFn, ptr5);

    Queue_remove(myQ);
    Queue_remove(myQ);
    //Queue_remove(myQ);
    //Queue_remove(myQ);
    //Queue_remove(myQ);

    Queue_add(myQ, ptr1);
    Queue_add(myQ, ptr2);
    Queue_add(myQ, ptr2);

    Queue_remove(myQ);
    Queue_remove(myQ);
    Queue_remove(myQ);


    printf("front val = %i, idx = %lu\n", *((int*)myQ->queue[myQ->front]), myQ->front);
    printf("back val = %i, idx = %lu\n", *((int*)myQ->queue[myQ->back]), myQ->back);

    printf("Element count = %lu\n", myQ->queueCount);
    
    // free the memory
    Queue_delete(myQ);

    return 0;
}