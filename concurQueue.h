
#include <stdlib.h>

// the matching function
typedef int (*Queue_matchFn)(void *userArg, void *ptr); // TODO make dummy for testing

struct Queue {
    size_t front; // index to front of queue
    size_t back; // index to back of queue
    void **queue; // pointer to alloc'd queue of void*
    size_t queueCount; // number of elements in the queue, i.e., queue spots filled
    size_t queueCap; // number of elements the queue can accomodate, i.e., queue capacity

    /******************************************************************************
    PER-QUEUE THREAD SYNCHRONIZATION 
    ******************************************************************************/

    pthread_mutex_t qLock;
    pthread_cond_t qCond;
    int writers; // condition to wake sleeping read threads
    int readers; // condition to wake sleeping write (add, remove) threads

    /*****************************************************************************/

};

// the queue constructor
struct Queue * Queue_new(size_t maxSize);

// the queue destructor
void Queue_delete(struct Queue *me);

// add data to the back of the queue
int Queue_add(struct Queue *me, void *data); /* Returns 0 if queue is full, 1 otherwise */

// remove data at the front of the queue
void *Queue_remove(struct Queue *me);    /* Returns NULL if queue is empty */

// returns pointer to element if userArg in queue, or NULL if not found
void * Queue_find(struct Queue *me, Queue_matchFn matchFn, void *userArg); 