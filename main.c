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
    Queue_matchFn comparator = &matchFn;

    // build a queue
    struct Queue *myQ = Queue_new(5);


    printf("front val = %i, idx = %lu\n", *((int*)myQ->queue[myQ->front]), myQ->front);
    printf("back val = %i, idx = %lu\n", *((int*)myQ->queue[myQ->back]), myQ->back);

    printf("Element count = %lu\n", myQ->queueCount);
    
    // free the memory
    Queue_delete(myQ);

    return 0;
}