#include "concurQueue.h"    // TODO needs to be a library
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>    


/* Queue constructor */
struct Queue * Queue_new(size_t maxSize) {

    // alloc queue struct
    struct Queue *myQ = calloc(1, sizeof(struct Queue));

    // alloc queue's actual queue of void*
    myQ->queue = malloc(sizeof(void*) * maxSize);

    // TODO handle this error case better
    if (myQ == NULL) {
        printf("Failed to malloc\n");
    } 

    // set thread synchronization members
    pthread_mutex_init(&myQ->qLock, NULL); // init with defaults
    pthread_cond_init(&myQ->qCond, NULL); // init with defaults
    myQ->writers = 0; // condition to wake sleeping read threads
    myQ->readers = 0; // condition to wake sleeping write (add, remove) threads
    
    myQ->queueCount = 0; // zero elements to start
    myQ->queueCap = maxSize; // queue's element capacity

    // init front and back to same queue index
    myQ->front = 0;
    myQ->back = 0;

    return myQ;

}

/* Queue destructor */
void Queue_delete(struct Queue *me) {

    // free all alloc'd memory!
    free(me->queue);
    
    // TODO handle errors for these destructions
    // must explicitly destroy dynamic mutex, cond var
    pthread_mutex_destroy(&me->qLock);
    pthread_cond_destroy(&me->qCond);
    free(me);

    return;

}

/*  
    Queue_add places data at BACK of queue
    returns 0 if queue is full, returns 1 if queue is not full 
    Uses 1 lock to protect shared resource (the queue) during add
*/
int Queue_add(struct Queue *me, void *data) {

    int result = 1; // assume queue is not full to start

    pthread_mutex_lock(&me->qLock);

    me->writers++;  // indicate a writing thread is present

    // if any threads are reading, sleep this thread
    while (me->readers) { 
        // wait unlocks mutex on sleep, re-locks on wake from broadcast
        pthread_cond_wait(&me->qCond, &me->qLock); 

        /* 
            on broadcast, all threads will wake and stay awake because readers
            will not increment until all waiting writers have had a go
            but, only the first will get the lock so they will be blocked here
            to wait or the unlock at the end of the working writer thread
        */ 
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

        me->queue[me->back] = data; // add element to queue
        me->queueCount++; // update element count

    }

    me->writers--;  // indicate a writer thread is done writing

    if (!me->writers) { // wake sleeping threads
        pthread_cond_broadcast(&me->qCond);
    }

    pthread_mutex_unlock(&me->qLock);

    return result;

}

/* 
    Queue_remove sets void* at front of queue to NULL
    returns void* to new front, which is NULL if queue is empty
    Uses 1 lock to protect share resource (the queue) during remove
*/
void *Queue_remove(struct Queue *me) {

    pthread_mutex_lock(&me->qLock);

    me->writers++;  // indicate a writing thread is present

    while (me->readers) {    // sleep if there are active reading threads
        pthread_cond_wait(&me->qCond, &me->qLock); // unlocks mutex on sleep, re-locks on wake

        /* 
            on broadcast, all threads will wake and stay awake because readers
            will not increment until all waiting writers have had a go
            but, only the first will get the lock so they will be blocked here
            to wait or the unlock at the end of the working writer thread
        */ 
    }
    
    // if not empty, remove front element, update count, update front index
    if (me->queueCount != 0) {
     
        me->queue[me->front] = NULL; // remove front element
        me->queueCount--; // decrement queue count

        // update front index to the new front element if queue is not now empty
        if (me->queueCount != 0) {

            me->front++; // go to next open spot

            // if front is out of bounds, wrap it around
            if (me->front == me->queueCap) {
                me->front = 0; // wrapround!
            }

        }
        
    }

    void* newFront = me->queue[me->front]; // is NULL if queue is empty

    me->writers--;  // indicate writer is complete

    if (!me->writers) { // wake all sleeping threads IFF no writers waiting
        pthread_cond_broadcast(&me->qCond); 
    }

    pthread_mutex_unlock(&me->qLock);

    // return pointer to front of queue - this will be NULL if queue is empty
    return newFront;

}

/*
    Queue_find searches queue for element matching userArg
    returns pointer to element if userArg in queue, or NULL if not found
    Uses 2 locks to allow reading thread to be blocked by add/remove
    but still allow concurrency of reading threads
*/
void * Queue_find(struct Queue *me, Queue_matchFn matchFn, void *userArg) {

    pthread_mutex_lock(&me->qLock); // LOCK

    /* 
            reader threads are blocked here if there are writers waiting
            otherwise, double lock implementation allows multiple reads
            and guarantees no deadlock for waiting threads
    */ 
    while (me->writers) {    // sleep until no writers
        pthread_cond_wait(&me->qCond, &me->qLock); // unlocks on sleep, re-locks on wake
    }

    me->readers++;  // indicate a reader thread present

    pthread_mutex_unlock(&me->qLock);   // UNLOCK

    //printf("Looking for %i... ", *((int*)userArg));

    size_t current = me->front;
    size_t count = me->queueCount;
    void* foundElement = NULL; // overwritten if element found

    // front to back for queue count, check queue for userArg
    while (count-- > 0) {

        // check if the current element is a match
        if (matchFn(userArg, me->queue[current]) == 0) {
            foundElement = me->queue[current];
            break; // terminate while loop to save time
        } 

        // update current to next element in queue
        current++;

        // wraparound current if needed
        if (current == me->queueCap) {

            current = 0; // wrapround!

        }

    }
    
    pthread_mutex_lock(&me->qLock);
    
    me->readers--;  // indicate a reader has finished

    if (!me->readers) { // wake all threads IFF all read threads done
        pthread_cond_broadcast(&me->qCond);
    }

    pthread_mutex_unlock(&me->qLock);

    return foundElement; // NULL or pointer to the found element

}