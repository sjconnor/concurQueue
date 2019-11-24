#include "concurQueue.h"    // TODO needs to be a library
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>    


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

    // set thread synchronization 
    pthread_mutex_init(&myQ->qLock, NULL); // init with defaults
    pthread_cond_init(&myQ->qCond, NULL); // init with defaults
    myQ->writers = 0; // condition to wake sleeping read threads
    myQ->readers = 0; // condition to wake sleeping write (add, remove) threads
    
    myQ->queueCount = 0; // zero elements to start
    myQ->queueCap = maxSize; // queue's element capacity

    // front and back start out at same index
    myQ->front = 0;
    myQ->back = 0;

    return myQ;

}

// queue destructor
void Queue_delete(struct Queue *me) {

    // free all alloc'd memory!
    free(me->queue);
    
    // TODO handle errors for these destructions
    pthread_mutex_destroy(&me->qLock);
    pthread_cond_destroy(&me->qCond);
    free(me);

    return;

}

/*  add data to BACK of queue, return 0 if queue is full, return 1 otherwise */
int Queue_add(struct Queue *me, void *data) {

    int result = 1; 

    pthread_mutex_lock(&me->qLock);

    me->writers++;  // indicate a writing thread is present

    while (me->readers) {    // SLEEP UNTIL NO READERS ACTIVE
        pthread_cond_wait(&me->qCond, &me->qLock); // unlocks on sleep, re-locks on wake
    }

    // check if queue is full
    if (me->queueCap == me->queueCount) {

        // TODO double check this later
        result = 0;

    } else if (me->queueCap < me->queueCount) {

        // TODO handle this error better
        printf("Error in queue add\n");
        result = -1;

    } else {

        // shift back by 1 element if not the first/only added to queue
        if (me->queueCount != 0) {

            me->back++; // go to next open spot
            
            // handle wraparound
            if (me->back == me->queueCap) {
                me->back = 0;
            }
        }

        me->queue[me->back] = data; // add element to the queue
        me->queueCount++; // update count of elements

    }

    me->writers--;  // indicate the writer is complete

    if (!me->writers) { // wake sleeping threads when write complete
        pthread_cond_broadcast(&me->qCond);
    }

    pthread_mutex_unlock(&me->qLock);

    return result;

}

// remove front of queue, return void* to new front, or NULL if empty */
void *Queue_remove(struct Queue *me) {

    pthread_mutex_lock(&me->qLock);

    me->writers++;  // indicate a writing thread is present

    while (me->readers) {    // SLEEP UNTIL NO READERS ACTIVE
        pthread_cond_wait(&me->qCond, &me->qLock); // unlocks on sleep, re-locks on wake
    }
    
    // if not empty, remove front element, update count, update front index
    if (me->queueCount != 0) {
     
        me->queue[me->front] = NULL; // erase front element
        me->queueCount--; // decrement queue count

        // update front index to the new front element if queue is not now empty
        if (me->queueCount != 0) {

            //printf("not empty yet\n");
            me->front++; // go to next open spot

            // if front is out of bounds, wrap it around
            if (me->front == me->queueCap) {
                printf("wrapping front!\n");
                me->front = 0; // wrapround!
            }

        }
        
    }

    void* newFront = me->queue[me->front]; // NULL if queue is empty

    me->writers--;  // indicate the writer is complete

    if (!me->writers) { // wake sleeping threads when write complete
        pthread_cond_broadcast(&me->qCond);
    }

    pthread_mutex_unlock(&me->qLock);

    // return pointer to front of queue - this will be NULL if queue is empty
    return newFront;

}

// returns pointer to element if userArg in queue, or NULL if not found
void * Queue_find(struct Queue *me, Queue_matchFn matchFn, void *userArg) {

    pthread_mutex_lock(&me->qLock); // LOCK

    while (me->writers) {    // SLEEP UNTIL NO WRITERS
        pthread_cond_wait(&me->qCond, &me->qLock); // unlocks on sleep, re-locks on wake
    }

    me->readers++;  // INCREMENT READER COND VAR

    pthread_mutex_unlock(&me->qLock);   // UNLOCK

    //printf("Looking for %i... ", *((int*)userArg));

    size_t current = me->front;
    size_t count = me->queueCount;
    void* foundElement = NULL; // will overwrite if element found

    // front to back, check the queue for the userArg element
    while (count-- > 0) {

        // check if the current element is a match
        if (matchFn(userArg, me->queue[current]) == 0) {
            //printf("\tFOUND!\n");
            foundElement = me->queue[current];
            break;
        }

        // update current to next element in queue
        current++;

        // wraparound current if needed
        if (current == me->queueCap) {
            //printf("wrapping current!\n");
            current = 0; // wrapround!
        }

    }
    

    pthread_mutex_lock(&me->qLock); // LOCK
    me->readers--;  // indicate a reader has finished

    if (!me->readers) { // wake all threads if all reads are done
        pthread_cond_broadcast(&me->qCond);
    }

    pthread_mutex_unlock(&me->qLock);// UNLOCK

    //printf("\tNOT FOUND :(\n");
    // element was not found in queue
    return foundElement; // NULL or pointer to the found element

}