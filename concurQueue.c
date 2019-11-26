#include "concurQueue.h"    // TODO needs to be a library
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>    
#include <unistd.h>
#include <errno.h>

/* 
    Queue constructor 
    returns pointer to alloc'd queue struct or NULL on error
    errno contains POSIX error if one occured
*/
struct Queue * Queue_new(size_t maxSize) {

    int rc = 0; // for error checking
    errno = 0;

    // alloc queue struct
    struct Queue *myQ = calloc(1, sizeof(struct Queue));

    if (!myQ) {
        return NULL;
    }

    // alloc queue's actual queue of void*
    myQ->queue = malloc(sizeof(void*) * maxSize);
    if (!myQ->queue) {
        fprintf(stderr, "error: malloc queue, rc: %d\n", rc);
        Queue_delete(myQ);
        errno = rc;
        return NULL;
    }

    // init thread synchronization members with defaults
    if((rc = pthread_mutex_init(&myQ->qLock, NULL))) { // returns 0 on success
        fprintf(stderr, "error: pthread_mutex_init, rc: %d\n", rc);
        Queue_delete(myQ); 
        errno = rc;
        return NULL;
    }
    if((rc = pthread_cond_init(&myQ->qCond, NULL))) { // returns 0 on success
        fprintf(stderr, "error: pthread_cond_init, rc: %d\n", rc);
        Queue_delete(myQ);
        errno = rc;
        return NULL;
    }

    myQ->writers = 0; // condition to wake sleeping read threads
    myQ->readers = 0; // condition to wake sleeping write (add, remove) threads
    myQ->queueCount = 0; // zero elements to start
    myQ->queueCap = maxSize; // queue's element capacity

    // init front and back to same queue index
    myQ->front = 0;
    myQ->back = 0;

    return myQ;

}

/* 
    Queue destructor 
    constraints: mutex must be unlocked and cond var not in use
*/
void Queue_delete(struct Queue *me) {

    free(me->queue);
    
    // must explicitly destroy dynamic mutex, cond var
    pthread_mutex_destroy(&me->qLock);
    pthread_cond_destroy(&me->qCond);

    free(me);

    return;

}

/*  
    Queue_add places data at BACK of queue
    returns 0 if queue is full, 1 if queue is not full, 
    returns -1 if error due to queue count somehow exceeding capacity,
    or returns -1 and errno set to POSIX failure
    Uses 1 lock to protect shared resource (the queue) during add
*/
int Queue_add(struct Queue *me, void *data) {

    int result = 1; // assume queue is not full to start
    int rc = 0; // for errors
    errno = 0;

    if ((rc = pthread_mutex_lock(&me->qLock))) { // 0 on success
        fprintf(stderr, "error: pthread_mutex_lock, rc: %d\n", rc);
        errno = rc;
        return -1;
    }

    printf("Add thread starting, ID: %lu\n", pthread_self());
    sleep(1); // SLEEP FOR TESTING

    me->writers++;  // indicate a writing thread is present

    // if any threads are reading, sleep this thread
    while (me->readers) { 

        // wait unlocks mutex on sleep, re-locks on wake from broadcast
        printf("\tthere are %d readers in line...\n", me->readers);
        printf("Add thread waiting, ID: %lu\n", pthread_self());
        if ((rc = pthread_cond_wait(&me->qCond, &me->qLock))) { // 0 on success
            fprintf(stderr, "error: pthread_cond_wait, rc: %d\n", rc);
            errno = rc;
            return -1;
        }   

        /* 
            on broadcast, all threads will wake and stay awake because readers
            will not increment until all waiting writers have had a go
            but, only the first will get the lock so they will be blocked here
            to wait or the unlock at the end of the working writer thread
        */ 
    }

    printf("Add thread WORKING, ID: %lu\n", pthread_self());


    // check if queue is full, else if error, else proceed
    if (me->queueCap == me->queueCount) {

        result = 0;

    } else if (me->queueCap < me->queueCount) {

        fprintf(stderr, "queue count exceeds capacity\n");
        return -1;

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
        printf("Add thread broadcasting, ID: %lu\n", pthread_self());

        if ((rc = pthread_cond_broadcast(&me->qCond))) { // 0 on success
            fprintf(stderr, "error: pthread_cond_broadcast, rc: %d\n", rc);
            errno = rc;
            return -1;
        }   

    }

    printf("Add thread finishing, ID: %lu\n", pthread_self());

    if ((rc = pthread_mutex_unlock(&me->qLock))) { // 0 on success
        fprintf(stderr, "error: pthread_mutex_unlock, rc: %d\n", rc);
        errno = rc;
        return -1;
    }  

    return result;

}

/* 
    Queue_remove sets void* at front of queue to NULL
    returns void* to removed element, or NULL if queue empty or error
    Uses 1 lock to protect share resource (the queue) during remove
*/
void *Queue_remove(struct Queue *me) {

    int rc = 0; // for error codes

    // TODO - need to handle errors some other way!

    if ((rc = pthread_mutex_lock(&me->qLock))) { // 0 on success
        fprintf(stderr, "error: pthread_mutex_lock, rc: %d\n", rc);
        errno = rc;
        return NULL; // TODO better error
    }

    printf("Remove thread starting, ID: %lu\n", pthread_self());
    sleep(1); // SLEEP FOR TESTING

    me->writers++;  // indicate a writing thread is present

    while (me->readers) {    // sleep if there are active reading threads

        printf("\tthere are %i readers in line\n", me->readers);
        printf("Remove thread waiting, ID: %lu\n", pthread_self());

        if ((rc = pthread_cond_wait(&me->qCond, &me->qLock))) { // 0 on success
            fprintf(stderr, "error: pthread_cond_wait, rc: %d\n", rc);
            errno = rc;
            return NULL; // TODO better error
        }   

        /* 
            on broadcast, all threads will wake and stay awake because readers
            will not increment until all waiting writers have had a go
            but, only the first will get the lock so they will be blocked here
            to wait or the unlock at the end of the working writer thread
        */ 
    }
    
    printf("Remove thread WORKING, ID: %lu\n", pthread_self());

    // store the element before removing for return - is NULL if empty queue
    void* oldFront = me->queue[me->front];

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

    me->writers--;  // indicate writer is complete

    if (!me->writers) { // wake all sleeping threads IFF no writers waiting

        printf("Remove thread broadcasting, ID: %lu\n", pthread_self());

        if ((rc = pthread_cond_broadcast(&me->qCond))) { // 0 on success
            fprintf(stderr, "error: pthread_cond_broadcast, rc: %d\n", rc);
            errno = rc;
            return NULL; // TODO better error 
        }  
    }


    if ((rc = pthread_mutex_unlock(&me->qLock))) { // 0 on success
        fprintf(stderr, "error: pthread_mutex_unlock, rc: %d\n", rc);
        errno = rc;
        return NULL; // TODO better error
    }

    // return pointer to removed element (NULL if queue was already empty)
    return oldFront;

}

/*
    Queue_find searches queue for element matching userArg
    returns pointer to element if userArg in queue
    returns NULL and errno = 0 if not found
    returns NULL and errno to POSIX error if mutex/condvar failure
    Uses 2 locks to allow reading thread to be blocked by add/remove
    but still allow concurrency of reading threads
*/
void * Queue_find(struct Queue *me, Queue_matchFn matchFn, void *userArg) {

    int rc = 0;
    errno = 0;

    if ((rc = pthread_mutex_lock(&me->qLock))) { // 0 on success
        fprintf(stderr, "error: pthread_mutex_lock, rc: %d\n", rc);
        errno = rc;
        return NULL;
    }
    
    printf("Find thread starting, ID: %lu\n", pthread_self());

    /* 
            reader threads are blocked here if there are writers waiting
            otherwise, double lock implementation allows multiple reads
            and guarantees no deadlock for waiting threads
    */ 
    while (me->writers) {    // sleep until no writers
        printf("\tthere are %d writers in line...\n", me->writers);
        printf("Find thread waiting, ID: %lu\n", pthread_self());

        if ((rc = pthread_cond_wait(&me->qCond, &me->qLock))) { // 0 on success
            fprintf(stderr, "error: pthread_cond_wait, rc: %d\n", rc);
            errno = rc;
            return NULL; // TODO better error
        }   

    }

    printf("Find thread WORKING, ID: %lu\n", pthread_self());

    me->readers++;  // indicate a reader thread present

    pthread_mutex_unlock(&me->qLock);   // UNLOCK

    sleep(1); // SLEEP FOR TESTING

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
    
    if ((rc = pthread_mutex_lock(&me->qLock))) { // 0 on success
        fprintf(stderr, "error: pthread_mutex_lock, rc: %d\n", rc);
        errno = rc;
        return NULL; // TODO better error
    }

    me->readers--;  // indicate a reader has finished

    if (!me->readers) { // wake all threads IFF all read threads done

        if ((rc = pthread_cond_broadcast(&me->qCond))) { // 0 on success
            fprintf(stderr, "error: pthread_cond_broadcast, rc: %d\n", rc);
            errno = rc;
            return NULL;
        }  

    }

    printf("Find thread finishing, ID: %lu\n", pthread_self());

    if ((rc = pthread_mutex_unlock(&me->qLock))) { // 0 on success
        fprintf(stderr, "error: pthread_mutex_unlock, rc: %d\n", rc);
        errno = rc;
        return NULL; // TODO better error
    }


    return foundElement; // NULL or pointer to the found element

}