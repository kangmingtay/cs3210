/*******************************************************************
* prod-con-threads.c
* Producer Consumer With C
* Compile: gcc -pthread -o prodcont prod-con-threads.c
* Run: ./prodcont
*******************************************************************/
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#define PRODUCERS 2
#define CONSUMERS 1
#define BUFFER_SIZE 10

int producer_buffer;
int consumer_sum;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

// Necessary Data Structures for Buffer
struct node {
    int val;
    struct node *next;
};
typedef struct node node;

struct queue {
    int count;
    node *front;
    node *rear;
};
typedef struct queue queue;

void initialize(queue *q) {
    q->count = 0;
    q->front = NULL;
    q->rear = NULL;
}

int isEmpty(queue *q) {
    return (q->rear == NULL);
}

void enqueue(queue *q, int val) {
    if (q -> count >= 10) {
        printf("Buffer is full!\n");    
    } else {
        node *tmp;
        tmp = malloc(sizeof(node));
        tmp -> val = val;
        tmp -> next = NULL;
    	if (!isEmpty(q)) {
    	    q -> rear -> next = tmp;
            q -> rear = tmp;
    	} else {
            q -> front = q -> rear = tmp;
    	}
    	q -> count++;
    }
}

int pop(queue *q) {
    node *tmp;
    int n = q -> front -> val;
    tmp = q -> front;
    q -> front = q -> front -> next;
    q -> count--;
    free(tmp);
    return n;
}

void printQueue(queue *q) {
    int size = q -> count;
    node *tmp;
    tmp = malloc(sizeof(node));
    tmp = q -> front;
    for (int i = 0; i < size; i++) {
        int val = tmp -> val;
    	printf("%d ", val);
        tmp = tmp -> next;
    }
}

// Create global buffer
queue *buffer;

void* producer(void* threadid)
{ 
    pthread_mutex_lock(&lock);
    
    if (producer_buffer < 10) {
	srand(time(NULL));
    	int num = rand() % 10 + 1;
        // add node to buffer
	enqueue(buffer, num);
    	producer_buffer++;
        // printQueue(buffer);
    }
    pthread_mutex_unlock(&lock);
}

void* consumer(void* threadid)
{
    // pthread_mutex_lock(&lock);
    // consume node from buffer
    while (1) {
    int num = pop(buffer);
    consumer_sum += num;
    producer_buffer--;
    }
    // printQueue(buffer);
    // pthread_mutex_unlock(&lock); 
}

int main(int argc, char* argv[])
{
    consumer_sum = 0;
    producer_buffer = 0;
    
    // initialize buffer
    buffer = malloc(sizeof(queue));
    initialize(buffer);

    // initialize array of threads for producers and consumers
    pthread_t producer_threads[PRODUCERS];
    pthread_t consumer_threads[CONSUMERS];
    long producer_threadid[PRODUCERS];
    long consumer_threadid[CONSUMERS];

    int rc;
    long t1, t2;
    for (t1 = 0; t1 < PRODUCERS; t1++) {
        int tid = t1;
        producer_threadid[tid] = tid;
        printf("creating producer %d\n", tid);
        rc = pthread_create(&producer_threads[tid], NULL, producer,
            (void*)producer_threadid[tid]);
        if (rc) {
            printf("Error: Return code from pthread_create() is %d\n", rc);
            exit(-1);
        }
    }

    for (t2 = 0; t2 < CONSUMERS; t2++) {
        int tid = t2;
        consumer_threadid[tid] = tid;
        printf("creating consumer %d\n", tid);
        rc = pthread_create(&consumer_threads[tid], NULL, consumer,
            (void*)consumer_threadid[tid]);
        if (rc) {
            printf("Error: Return code from pthread_create() is %d\n", rc);
            exit(-1);
        }
    }
   
    // join other threads to main thread 
    for (int i = 0; i < PRODUCERS; i++) {
    	pthread_join(producer_threads[i], NULL);
    }
    
    for (int i = 0; i < CONSUMERS; i++) {
    	pthread_join(consumer_threads[i], NULL);
    }

    printf("### consumer_sum final value = %d ###\n",
        consumer_sum);
    pthread_exit(NULL);
}
