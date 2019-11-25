
#include <stdlib.h>

// the matching function
typedef int (*Queue_matchFn)(void *userArg, void *ptr); // TODO make dummy for testing

struct Queue {

    pthread_t tID[3]; // for testing

    size_t front; // index to front of queue
    size_t back; // index to back of queue
    void **queue; // pointer to alloc'd queue of void*
    size_t queueCount; // elements in queue, i.e., queue spots filled
    size_t queueCap; // elements queue can accomodate, i.e., queue capacity

    pthread_mutex_t qLock; // mutex to block threads
    pthread_cond_t qCond; // condition variable to wake sleeping threads

    int writers; // condition indicating how many threads want to write
    int readers; // condition indicating how many threads want to read


};

// Queue constructor
struct Queue * Queue_new(size_t maxSize);

// Queue destructor
void Queue_delete(struct Queue *me);

// Add data to back of queue, returns 0 if queue full (add failed), else 1
int Queue_add(struct Queue *me, void *data); 

// Remove data at front of queue, returns NULL if empty queue
void *Queue_remove(struct Queue *me);    

// Returns pointer to element if userArg in queue, or NULL if not found
void * Queue_find(struct Queue *me, Queue_matchFn matchFn, void *userArg); 