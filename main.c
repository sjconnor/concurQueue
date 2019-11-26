#include "concurQueue.h"    // TODO needs to be a library
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>


// for multithread testing of find 
struct findArgs {
   struct Queue *q;
   Queue_matchFn fn;
   void *arg;
};

// for multithread testing of add
struct addArgs {
   struct Queue *q;
   void *arg;
};

// for multithread testing of remove
struct removeArgs {
   struct Queue *q;
};

// Just a dummy comparator for testing - WORKS ON INT ONLY!
int matchFn(void *userArg, void *ptr) {

    return *(int*)userArg - *(int*)ptr; 

}

// does basic test of adding a large number of elements
void testManyInts(void) {

    printf("TEST 1: Adding elements...\n");

    int result = 0;
    const int numElements = 1024;

    struct Queue *myQ = Queue_new(numElements); 

    int intArray[numElements];  // some test elements
    
    // add numElements to the queue
    for (int i = 0; i < numElements; i++) {
        intArray[i] = i;
        Queue_add(myQ, (void*)&intArray[i]);
    }

    // try to exceed capacity
    if (Queue_add(myQ, (void*)&intArray[0])) {
        printf("\tFAIL: added to a full queue\n");
    } else {
        printf("\tPASS: add to full queue rejected\n");
    }

    // check element count
    if (myQ->queueCount != numElements) {
        printf("\tFAIL: queue count incorrect\n");
        result = -1;
    } else {
        printf("\tPASS: queue count correct\n");
    }
    // check front
    if (*((int*)myQ->queue[myQ->front]) != intArray[0]) {
        printf("\tFAIL: front element incorrect\n");
        result = -1;
    } else {
        printf("\tPASS: front element correct\n");
    }
    // check back
    if (*((int*)myQ->queue[myQ->back]) != intArray[numElements - 1]) {
        printf("\tFAIL: back element incorrect\n");
        result = -1;
    } else {
        printf("\tPASS: back element correct\n");
    }

    int findCheck = 0; // use for single pass/fail for all finds

    for (int i = 0; i < numElements; i++) {
        if(Queue_find(myQ, &matchFn, (void*)&intArray[i]) == NULL) {
            printf("\tFAIL: didn't find element %i\n", i);
            result = -1;
            findCheck = -1;
        }
    }

    if (!findCheck) {
        printf("\tPASS: all elements found\n");
    }

    int badNum = -5;

    // try to find an element not in there
    if (Queue_find(myQ, &matchFn, (void*)&badNum) != NULL) {
        printf("\tFAIL: didn't return NULL on find of nonexistent element\n");
    } else {
        printf("\tPASS: didn't find nonexistent element\n");
    }

    // free the memory
    Queue_delete(myQ);

    if (result) {
         printf("TEST 1 RESULT: FAIL\n\n"); 
    } else {
        printf("TEST 1 RESULT: PASS\n\n");
    }

    return;

}

// tests basic add, remove, and find on small number of elements
void testRemove(void) {

    printf("TEST 2: Removing elements...\n");

    int result = 0;
    const int numElements = 5;

    struct Queue *myQ = Queue_new(numElements); 

    int intArray[numElements];
    
    // add numElements to the queue
    for (int i = 0; i < numElements; i++) {
        intArray[i] = i;
        Queue_add(myQ, (void*)&intArray[i]);
    }

    void* removed; // remove should return pointer to new front of queue

    // remove all elements and check 
    for (int i = 0; i < numElements; i++) {

        //currentFront 
        removed = Queue_remove(myQ);

        //printf("\tremoved %d\n", *(int*)removed);

        // check for NULL on nonempty queue and correct value if not NULL
        if (removed == NULL) { 
            printf("\tFAIL: returned NULL on nonempty queue\n");
            result = -1;
        } else if (*(int*)removed != i) {
            printf("\tFAIL: removed = %d, expected = %d\n", *(int*)removed, i);
            result = -1;
        }
        
        // check that queueCount updated
        if (myQ->queueCount != numElements - i - 1) { 
            printf("\tFAIL: queueCount = %lu, expected %i\n", myQ->queueCount, numElements-i-1);
            result = -1;
        }
        
        // check that front updated to next in line, or NULL if last element was removed
        if (myQ->queueCount > 0 && (*((int*)myQ->queue[myQ->front]) != i + 1 || myQ->front != i + 1)) {
            printf("\tFAIL: front of queue not updated correctly\n");
            result = -1;
        } else if (myQ->queueCount == 0 && (myQ->queue[myQ->front] != NULL)) { 
            printf("\tFAIL: empty queue front is not NULL\n");
            result = -1;
        }
        

    }

    // test removal on empty queue
    removed = Queue_remove(myQ);

    if (removed == NULL) {
        printf("\tPASS: removal on empty queue returned NULL\n");
    } else {
        printf("\tFAIL: removal on empty queue returned non-NULL\n");
        result = -1;
    }

    // single pass/fail for removals up to last element
    if (!result) {
        printf("\tPASS: removal of 0...n-1 elements all correct\n");
    } else {
        printf("\tFAIL: one or more errors in removing elements\n");
    }

    Queue_delete(myQ);

    if (result) {
        printf("TEST 2 RESULT: FAIL\n\n"); 
    } else {
        printf("TEST 2 RESULT: PASS\n\n");
    }

    return;

}

// tests basic operations to show queue circularity
void testCirc() {

    printf("TEST 3: Testing circular-ness and intermingled functionality\n");

    int result = 0;

    const int numElements = 5;

    // build a queue
    struct Queue *myQ = Queue_new(numElements); 

    // some test elements
    int intArray[numElements];
    
    // add numElements to the queue
    for (int i = 0; i < numElements; i++) {
        intArray[i] = i;
        Queue_add(myQ, (void*)&intArray[i]);
    }

    // remove numElements so further adds guarantee wraparound
     for (int i = 0; i < numElements; i++) {
        Queue_remove(myQ);
    }

    // add numElements back into the queue
    for (int i = 0; i < numElements; i++) {
        intArray[i] = i;
        Queue_add(myQ, (void*)&intArray[i]);
    }

    // front incorrect
    if (myQ->front != numElements - 1 || *((int*)myQ->queue[myQ->front]) != intArray[0]) {
        printf("\tFAIL: front incorrect after wraparound\n");
        result = -1;
    } else {
        printf("\tPASS: front correct after wraparound\n");   
    }

    // back incorrect
    if (myQ->back != numElements - 2 || *((int*)myQ->queue[myQ->back]) != intArray[numElements-1]) {
        printf("\tFAIL: back incorrect after wraparound\n");
        result = -1;
    } else {
        printf("\tPASS: back correct after wraparound\n");   
    }

    Queue_delete(myQ);

    if (result) {
         printf("TEST 3 RESULT: FAIL\n\n"); 
    } else {
        printf("TEST 3 RESULT: PASS\n\n");
    }

    return;
}

// TODO better error handle
// called by pthread_create for testing 
void* findThreadPayload(void *args) {

    struct findArgs *a = args;
    printf("\t%lu doing find\n", pthread_self());
    return Queue_find(a->q, a->fn, a->arg);
    

}

// TODO better error handle
// called by pthread_create for testing 
void* addThreadPayload(void *args) {
    
    struct addArgs *a = args;
    printf("\t%lu doing add\n", pthread_self());
    
    if(Queue_add(a->q, a->arg) == -1) {
        printf("something went wrong\n");
    }

    return NULL;

}

// TODO better error handle
// called by pthread_create for testing
void* removeThreadPayload(void *args) {

    struct removeArgs *a = args;
    printf("\t%lu doing remove\n", pthread_self());

    Queue_remove(a->q);

    return NULL;

}

void testThreads1() {

    pthread_t tID[10]; 

    int result = 0;
    int rc; // for error checks

    const int numElements = 10;

    // build a queue
    struct Queue *myQ = Queue_new(numElements); 

    // some test elements
    int intArray[numElements];
    
    // add numElements to the queue
    for (int i = 0; i < numElements; i++) {
        intArray[i] = i;
        Queue_add(myQ, (void*)&intArray[i]);
    }

    /**********************************
    Test for concurrent reads
    ***********************************/

    printf("TEST 4... concurrent reads\n");

    struct findArgs fArgs;
    fArgs.q = myQ;
    fArgs.fn = &matchFn;
    fArgs.arg = (void*)&intArray[numElements-1];

    // time how long it takes 10 threads to do find()
    struct timespec start, finish;
    double elapsed;

    clock_gettime(CLOCK_MONOTONIC, &start);

    // create 10 threads to execute find
    for (int i = 0; i < 10; i++) {

        if ((rc = pthread_create(&(tID[i]), NULL, findThreadPayload, &fArgs))) {
            fprintf(stderr, "error: pthread_create, rc: %d\n", rc);
            return;
        }
         
    }

    // join all threads before continuing
    for (int i = 0; i < 10; i++) {
        pthread_join(tID[i], NULL);
    }

    // if it took < 10 seconds, concurrency must have happened due to sleep
    clock_gettime(CLOCK_MONOTONIC, &finish);
    elapsed = (finish.tv_sec - start.tv_sec);
    elapsed += (finish.tv_nsec - start.tv_nsec) / 1000000000.0;

    if (elapsed > 10) {
        printf("\tFAIL: it took %f seconds to do 10 finds\n", elapsed);
        result = -1;
    } else {
        printf("\tPASS: it took %f seconds to do 10 finds\n", elapsed);
    }

    if (result) {
         printf("TEST 4 RESULT: FAIL\n\n"); 
    } else {
        printf("TEST 4 RESULT: PASS\n\n");
    }

    Queue_delete(myQ);

    return;

}

void testThreads2() {

    pthread_t tID[10]; 

    int result = 0;
    int rc; // for error checks

    const int numElements = 10;

    // build a queue
    struct Queue *myQ = Queue_new(numElements); 

    // some test elements
    int intArray[numElements];
    
    // add numElements to the queue
    for (int i = 0; i < numElements; i++) {
        intArray[i] = i;
        Queue_add(myQ, (void*)&intArray[i]);
    }

    /**********************************
    Test for read-write blocking
    ***********************************/

    printf("TEST 5... read-write blocking\n");

    struct addArgs aArgs;
    aArgs.q = myQ;
    aArgs.arg = (void*)&intArray[numElements-1];

    struct removeArgs rArgs;
    rArgs.q = myQ;

    struct findArgs fArgs;
    fArgs.q = myQ;
    fArgs.fn = &matchFn;
    fArgs.arg = (void*)&intArray[numElements-1];

    // time how long it takes 10 threads to do find()
    struct timespec start, finish;
    double elapsed;

    clock_gettime(CLOCK_MONOTONIC, &start);

    // create 10 threads to execute a mix of functions
    for (int i = 0; i < 10; i++) {

        if (i % 3 == 0) {
            //printf("\tadd %d\n", i);
            if ((rc = pthread_create(&(tID[i]), NULL, addThreadPayload, &aArgs))) {
                fprintf(stderr, "error: pthread_create, rc: %d\n", rc);
                return;
            }
        } else if (i % 2 == 0) { // 0, 2, 4, 8
            //printf("\tremove %d\n", i);
            if ((rc = pthread_create(&(tID[i]), NULL, removeThreadPayload, &rArgs))) {
                fprintf(stderr, "error: pthread_create, rc: %d\n", rc);
                return;
            }
        } else { 
            //printf("\tfind %d\n", i);
            if ((rc = pthread_create(&(tID[i]), NULL, findThreadPayload, &fArgs))) {
                fprintf(stderr, "error: pthread_create, rc: %d\n", rc);
                return;
            }   
        }
         
    }

    // join all threads before continuing
    for (int i = 0; i < 10; i++) {
        pthread_join(tID[i], NULL);
    }

    // if it took < 10 seconds, concurrency must have happened due to sleep
    clock_gettime(CLOCK_MONOTONIC, &finish);
    elapsed = (finish.tv_sec - start.tv_sec);
    elapsed += (finish.tv_nsec - start.tv_nsec) / 1000000000.0;

    if (elapsed < 7) {
        printf("\tFAIL: it took %f seconds to do 7 writes and 3 reads\n", elapsed);
        result = -1;
    } else {
        printf("\tPASS: it took %f seconds to do 7 writes and 3 reads\n", elapsed);
    }

    if (result) {
         printf("TEST 5 RESULT: FAIL\n\n"); 
    } else {
        printf("TEST 5 RESULT: PASS\n\n");
    }

    Queue_delete(myQ);

    return;

}

int main() {

    testManyInts();
    testRemove();
    testCirc();

    /* 
        uncomment sleep(x) code in concurQueue.c for thread tests (they use timing)
        uncomment areas marked FOR TIMED TESTS to inspect results with printf
        without this, testThreads results will be inaccurate!
    */
    
    testThreads1(); // tests that reads are concurrent 
    testThreads2(); // tests read-write blocking

    return 0;
}