#include "concurQueue.h"    // TODO needs to be a library
#include <stdio.h>
#include <stdlib.h>

// the matching function
//typedef int (*Queue_matchFn)(void *userArg, void *ptr); // TODO make dummy for testing

// the queue constructor
struct Queue * Queue_new(size_t maxSize) {

    // alloc the queue struct
    struct Queue *myQ = calloc(1, sizeof(struct Queue));

    // alloc the queue data
    myQ->queue = malloc(sizeof(void*) * maxSize);

    // TODO handle this error case better
    if (myQ == NULL) {
        printf("Failed to malloc\n");
    } 
    
    myQ->queueCount = 0; // zero elements to start
    myQ->queueCap = maxSize; // queue can hold this much

    // front and back start out as the same
    myQ->front = 0;
    myQ->back = 0;

    return myQ;

}

// the queue destructor
void Queue_delete(struct Queue *me) {

    // free all alloc'd memory!
    free(me->queue);
    free(me);

    return;

}

// add data to the back of the queue
/* Returns 0 if queue is full, 1 otherwise */
int Queue_add(struct Queue *me, void *data) {

    // check if queue is full
    if (me->queueCap == me->queueCount) {
        // TODO double check this later
        printf("Queue is full\n");
        return 0;
    } else if (me->queueCap < me->queueCount) {
        // TODO handle this error better
        printf("Error in queue add\n");
        return -1;
    }

    // shift back by 1 element if not the first/only added to queue
    if (me->queueCount != 0) {
        //printf("not first\n");
        me->back++; // go to next open spot
        
        // TODO handle wraparound
        if (me->back == me->queueCap) {
            me->back = 0; // wrapround!
        }
    }

    me->queue[me->back] = data;

    printf("\tadded %i at idx %lu\n", *((int*)me->queue[me->back]), me->back);
    
    // update queue count
    me->queueCount++; // update count of elements
    //printf("\tQUEUE COUNT = %lu\n", me->queueCount);

    return 1;

}

// remove data at the front of the queue
/* Returns NULL if queue is empty */
void *Queue_remove(struct Queue *me) {

    printf("\tREMOVING front...\n");
    
    // if queue is empty
    if (me->queueCount == 0) {
        printf("\tnothing to remove\n");
        return NULL;
    }

    printf("\told front = %i\n", *((int*)me->queue[me->front]));

    me->queue[me->front] = NULL; // erase front element    
    me->queueCount--; // decrement queue count

    // shift up by 1 element if not empty
    if (me->queueCount != 0) {
        printf("not empty yet\n");
        me->front++; // go to next open spot

        // TODO handle wraparound
        if (me->front == me->queueCap) {
            printf("wrapping front!\n");
            me->front = 0; // wrapround!
        }

    } else {
        printf("queue is empty\n");
    }


    //printf("\tQUEUE COUNT = %lu\n", me->queueCount);

    return me->queue[me->front]; // NULL if removal emptied queue

}

// returns pointer to element if userArg in queue, or NULL if not found
void * Queue_find(struct Queue *me, Queue_matchFn matchFn, void *userArg) {

    return NULL;

}
