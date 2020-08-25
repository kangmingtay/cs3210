/*******************************************************************
* race-condition.c
* Demonstrates race condition.
* Compile: gcc -pthread -o race race-condition.c
* Run: ./race
*******************************************************************/
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#define ADD_THREADS 4
#define SUB_THREADS 4
#define COUNT_LIMIT 4

int global_counter;
int count_add = 0;
pthread_mutex_t lock;
pthread_cond_t count_additions;

void* add(void* threadid)
{
    pthread_mutex_lock(&lock);
    long tid;

    tid = (long)threadid;
    global_counter++;
    sleep(rand() % 2);
    printf("add thread #%ld incremented global_counter!\n", tid);
    count_add++;
    if (count_add == COUNT_LIMIT) {
	printf("add(): thread %ld, count_add = %d Threshold reached. ", (long)threadid, count_add);
	pthread_cond_signal(&count_additions);
	printf("Just sent signal.\n");
    }
    pthread_mutex_unlock(&lock);
}

void* sub(void* threadid)
{
    pthread_mutex_lock(&lock);
    while (count_add < COUNT_LIMIT) {
	printf("sub(): thread %ld count_add = %d. Going into wait...\n", (long)threadid, count_add);
    	pthread_cond_wait(&count_additions, &lock);
	printf("sub(): thread %ld Condition signal received. count_add = %d\n", (long)threadid, count_add);
    }
    long tid;

    tid = (long)threadid;
    global_counter--;
    sleep(rand() % 2);
    printf("sub thread #%ld deducted global_counter! \n", tid);
    pthread_mutex_unlock(&lock);
}

int main(int argc, char* argv[])
{
    global_counter = 10;
    pthread_t add_threads[ADD_THREADS];
    pthread_t sub_threads[SUB_THREADS];
    long add_threadid[ADD_THREADS];
    long sub_threadid[SUB_THREADS];

    pthread_mutex_init(&lock, NULL);
    pthread_cond_init(&count_additions, NULL);

    int rc;
    long t1, t2;
    for (t1 = 0; t1 < ADD_THREADS; t1++) {
        int tid = t1;
        add_threadid[tid] = tid;
        printf("main thread: creating add thread %d\n", tid);
        rc = pthread_create(&add_threads[tid], NULL, add,
            (void*)add_threadid[tid]);
        if (rc) {
            printf("Return code from pthread_create() is %d\n", rc);
            exit(-1);
        }
    }

    for (t2 = 0; t2 < SUB_THREADS; t2++) {
        int tid = t2;
        sub_threadid[tid] = tid;
        printf("main thead: creating sub thread %d\n", tid);
        rc = pthread_create(&sub_threads[tid], NULL, sub,
            (void*)sub_threadid[tid]);
        if (rc) {
            printf("Return code from pthread_create() is %d\n", rc);
            exit(-1);
        }
    }

    printf("### global_counter final value = %d ###\n",
        global_counter);
    pthread_mutex_destroy(&lock);
    pthread_cond_destroy(&count_additions);
    pthread_exit(NULL);
}
