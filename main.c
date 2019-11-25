#include "concurQueue.h"    // TODO needs to be a library
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>


struct findArgs {
   struct Queue *q;
   Queue_matchFn fn;
   void *arg;
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

    const int badNum = -5;

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

    void* newFront; // remove should return pointer to new front of queue

    // remove all but one element
    for (int i = 0; i < numElements - 1; i++) {

        newFront = Queue_remove(myQ);

        // check for NULL on nonempty queue
        if (newFront == NULL) { 
            //printf("\tFAIL: returned NULL on nonempty queue\n");
            result = -1;
        }
        
        // check that queueCount updated
        if (myQ->queueCount != numElements - i - 1) { 
            //printf("\tFAIL: queueCount = %lu, expected %i\n", myQ->queueCount, numElements-i-1);
            result = -1;
        } //else {
            //printf("\tPASS: queueCount = %lu, expected %i\n", myQ->queueCount, numElements-i-1);
        //}
        
        // check that front updated
        if (*((int*)myQ->queue[myQ->front]) != i + 1 || myQ->front != i + 1) {
            //printf("\tFAIL: front of queue not updated correctly\n");
            result = -1;
        } //else {
            //printf("front = %i, front idx = %lu\n", *((int*)myQ->queue[myQ->front]), myQ->front);
            //printf("\tPASS: front idx and front element updated correctly\n");
        //}

    }

    // single pass/fail for removals up to last element
    if (!result) {
        printf("\tPASS: removal of 0...n-1 elements done correctly\n");
    } else {
        printf("\tFAIL: removal of 0...n-1 elements incorrectly done\n");
    }

    // removal of last element
    if (Queue_remove(myQ) == NULL && myQ->queueCount == 0 && myQ->front == myQ->back) {
        printf("\tPASS: removal of last element done correctly\n");
    } else {
        printf("\tFAIL: removal of last element incorrectly done\n");
        result = -1;
    }

    // removal on empty queue
    if (Queue_remove(myQ) != NULL) {
        printf("\tFAIL: removal on empty queue returned non-NULL\n");
    } else {
        printf("\tPASS: removal on empty queue returned NULL\n");
    }

    Queue_delete(myQ);

    if (result) {
         printf("TEST 2 RESULT: FAIL\n\n"); 
    } else {
        printf("TEST 2 RESULT: PASS\n\n");
    }

    return;

}

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

// because pthread_create doesn't work for functions with > 1 param
void* findThreadPayload(void *args) {

    struct findArgs *a = args;
    return Queue_find(a->q, a->fn, a->arg);

}

void testThreads() {

    printf("TEST 4... multithreading\n");

    int result = 0;

    const int numElements = 5;

    // build a queue
    struct Queue *myQ = Queue_new(numElements); 

    printf("\n\t THERE ARE %d writers in the queue\n\n", myQ->writers);

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

    
    printf("\n\nmy ID is %lu\n\n", pthread_self());


    struct findArgs args;
    args.q = myQ;
    args.fn = &matchFn;
    args.arg = (void*)&intArray[1];

    // create 3 threads to execute find
    for (int i = 0; i < 3; i++) {

        pthread_create(&(myQ->tID[i]), NULL, findThreadPayload, &args);
         
    }
    
    pthread_join(myQ->tID[0], NULL);
    pthread_join(myQ->tID[1], NULL);
    pthread_join(myQ->tID[2], NULL);


    /***********************************
    Test for !(find && add)
    ***********************************/

    /***********************************
    Test for !(find && remove)
    ***********************************/

    /***********************************
    Test for !(add && remove)
    ***********************************/

    /***********************************
    Test for !(add && add)
    ***********************************/

    /***********************************
    Test for !(remove && remove)
    ***********************************/

    if (result) {
         printf("TEST 4 RESULT: FAIL\n\n"); 
    } else {
        printf("TEST 4 RESULT: PASS\n\n");
    }

    Queue_delete(myQ);

    return;

}

int main() {

    printf("You can do it!\n");

    //testManyInts();
    //testRemove();
    //testCirc();

    testThreads();

    return 0;
}