#include "concurQueue.h"    // TODO needs to be a library
#include <stdio.h>
#include <stdlib.h>

// queue constructor
struct Queue * Queue_new(size_t maxSize) {

    // alloc queue struct
    struct Queue *myQ = calloc(1, sizeof(struct Queue));

    // alloc queue's actual queue of void*
    myQ->queue = malloc(sizeof(void*) * maxSize);

    // TODO handle this error case better
    if (myQ == NULL) {
        printf("Failed to malloc\n");
    } 
    
    myQ->queueCount = 0; // zero elements to start
    myQ->queueCap = maxSize; // queue's element capacity

    // front and back start out at same index
    myQ->front = 0;
    myQ->back = 0;

    return myQ; // return pointer to the queue struct

}

// queue destructor
void Queue_delete(struct Queue *me) {

    // free all alloc'd memory!
    free(me->queue);
    free(me);

    return;

}

/*  add data to BACK of queue, return 0 if queue is full, return 1 otherwise */
int Queue_add(struct Queue *me, void *data) {

    // check if queue is full
    if (me->queueCap == me->queueCount) {
        // TODO double check this later
        //printf("Queue is full\n");
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

    //printf("\tadded %i at idx %lu\n", *((int*)me->queue[me->back]), me->back);
    
    // update queue count
    me->queueCount++; // update count of elements
    //printf("\tQUEUE COUNT = %lu\n", me->queueCount);

    return 1;

}

// remove front of queue, return void* to new front, or NULL if empty */
void *Queue_remove(struct Queue *me) {

    //printf("\tREMOVING front...\n");
    
    // if queue is already empty
    if (me->queueCount == 0) {
        //printf("nothing to remove\n");
        return NULL;
    }

    //printf("\told front = %i\n", *((int*)me->queue[me->front]));

    me->queue[me->front] = NULL; // erase front element
    me->queueCount--; // decrement queue count

    // update front index to the new front element if queue is not empty
    if (me->queueCount != 0) {

        //printf("not empty yet\n");
        me->front++; // go to next open spot

        // if front is out of bounds, wrap it around
        if (me->front == me->queueCap) {
            printf("wrapping front!\n");
            me->front = 0; // wrapround!
        }

    }

    //printf("\tQUEUE COUNT = %lu\n", me->queueCount);

    // return pointer to front of queue - this will be NULL if queue is empty
    return me->queue[me->front];

}

// returns pointer to element if userArg in queue, or NULL if not found
void * Queue_find(struct Queue *me, Queue_matchFn matchFn, void *userArg) {

    //printf("Looking for %i... ", *((int*)userArg));

    size_t current = me->front;
    size_t count = me->queueCount;

    // front to back, check the queue for the userArg element
    while (count-- > 0) {

        // check if the current element is a match
        if (matchFn(userArg, me->queue[current]) == 0) {
            //printf("\tFOUND!\n");
            return me->queue[current];
        }

        // update current to next element in queue
        current++;

        // wraparound current if needed
        if (current == me->queueCap) {
            //printf("wrapping current!\n");
            current = 0; // wrapround!
        }

    }
    

    //printf("\tNOT FOUND :(\n");
    // element was not found in queue
    return NULL;

}