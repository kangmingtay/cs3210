/*******************************************************************
* threads.c
* Demonstrates thread creation and termination.
* Compile: gcc -pthread -o threads threads.c
* Run: ./threads
******************************************************************/
#include <pthread.h> // including pthread library
#include <stdio.h>
#include <stdlib.h>
#define NUM_THREADS 8

int counter = 0;

void* work(void* threadid) // function to run in parallel
{
    long tid;
    tid = (long)threadid;
    counter++;
    printf("thread #%ld incrementing counter. counter = %d\n", tid, counter);
    pthread_exit(NULL); // terminating thread
}

int main(int argc, char* argv[])
{
    pthread_t threads[NUM_THREADS]; // reference to threads
    int rc;
    long t;
    for (t = 0; t < NUM_THREADS; t++) {
        printf("main thead: creating thread %ld\n", t);

        // pthread_create spawns a new thread and return 0 on success
        rc = pthread_create(&threads[t], NULL, work, (void*)t);
        if (rc) {
            printf("Return code from pthread_create() is %d\n", rc);
            exit(-1);
        }
    }
    pthread_exit(NULL);
}
