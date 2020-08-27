/*******************************************************************
* prod-con-threads.c
* Producer Consumer With C
* Compile: gcc -pthread -o prodcont prod-con-threads.c
* Run: ./prodcont
*******************************************************************/
#include <pthread.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#define PRODUCERS 2
#define CONSUMERS 1
#define BUFFER_SIZE 10
#define THRESHOLD 100

int producer_buffer[BUFFER_SIZE] = {};
int consumer_sum;
int read_i;
int write_i;
int counter;
pthread_mutex_t lock;
pthread_mutex_t items;
pthread_mutex_t spaces;
pthread_cond_t items_threshold_cv;
pthread_cond_t spaces_threshold_cv;

bool isQueueFull() {
    return producer_buffer[write_i] != 0; // if 1, queue is full
}

bool isQueueEmpty() {
    return producer_buffer[read_i] == 0; // if 1, queue is empty
}

void printBuffer() {
	for (int i = read_i; i < BUFFER_SIZE + read_i; i++) {
		if (producer_buffer[i % BUFFER_SIZE] == 0) {
			printf("_ ");
		} else {
			printf("%d ", producer_buffer[i % BUFFER_SIZE]);
		}
	}
	printf("\n");
}

void push(int n)
{
    if (producer_buffer[write_i] == 0) {
        producer_buffer[write_i] = n;
        write_i = (write_i + 1) % BUFFER_SIZE;
    }
    else {
        // printf("Buffer is full!\n");
    }
}

int pop()
{
    int num = producer_buffer[read_i];
    producer_buffer[read_i] = 0;
    read_i = (read_i + 1) % BUFFER_SIZE;

    return num;
}

void* producer(void* threadid)
{	
	
	while(1) {
        pthread_mutex_lock(&spaces);
        if (isQueueFull()) {
            printf("Buffer is full! Waiting for consumer to read!\n");
            pthread_cond_wait(&spaces_threshold_cv, &spaces);
		} 
        if (counter == THRESHOLD) {
            printf("=== Threshold is reached ===\n");
            pthread_mutex_unlock(&spaces);
            break;
        }

        // Critical Section to modify buffer
		pthread_mutex_lock(&lock);

		int num = (rand() % 10) + 1;
		printf("Writing %d to buffer: ", num);
		push(num);	
		printBuffer();
		pthread_mutex_unlock(&lock);
        // End of critical section

		pthread_mutex_unlock(&spaces);
        // signal for consumer to read
        printf("Sending signal to consumer!\n");
        pthread_cond_signal(&items_threshold_cv);

		// sleep(1);
	}
	pthread_exit(NULL);
}

void* consumer(void* threadid)
{
	while (1) {
        pthread_mutex_lock(&items);
        if (isQueueEmpty()) {
            // Wait for producer to signal that there is at least an item in the queue
            printf("Buffer is empty! Waiting for producer to produce...\n");
            // Waits and gives up lock?
            pthread_cond_wait(&items_threshold_cv, &items);
		}

        if (counter == THRESHOLD) {
            printf("=== Threshold is reached ===\n");
            pthread_mutex_unlock(&items);
            break;
        }

		pthread_mutex_lock(&lock);
        int num = pop();
        printf("Reading %d from buffer: ", num);
        printBuffer();
        consumer_sum += num;
        counter += 1;
		pthread_mutex_unlock(&lock);
        pthread_mutex_unlock(&items);

        printf("Finished reading! Sending signal to producer...\n");
        pthread_cond_signal(&spaces_threshold_cv);
		// sleep(1);
	}
	pthread_exit(NULL);
}

int main(int argc, char* argv[])
{	
	read_i = 0;
	write_i = 0;
    counter = 0;

    pthread_mutex_init(&lock, NULL);
    pthread_mutex_init(&items, NULL);
    pthread_mutex_init(&spaces, NULL);
    pthread_cond_init(&items_threshold_cv, NULL);
    pthread_cond_init(&spaces_threshold_cv, NULL);

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

    printf("### counter final value = %d ###\n",
        counter);
    printf("### consumer_sum final value = %d ###\n",
        consumer_sum);
    
    pthread_exit(NULL);
}
