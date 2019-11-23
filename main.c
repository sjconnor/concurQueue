#include "concurQueue.h"    // TODO needs to be a library
#include <stdio.h>
#include <stdlib.h>

int main() {

    printf("You can do it!\n");

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