/*******************************************************************
* prod-con-threads.c
* Producer Consumer With C
* Compile: gcc -pthread -o prodcont prod-con-threads.c
* Run: ./prodcont
*******************************************************************/
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <semaphore.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/wait.h>

#define PRODUCERS 2
#define CONSUMERS 1
#define BUFFER_SIZE 10

// int read_i;
// int write_i;

int producer_buffer[BUFFER_SIZE] = {};
int consumer_sum;

bool isQueueFull(int* buffer, int* write_i) {
    return buffer[*write_i] != 0; // if 1, queue is full
}

bool isQueueEmpty(int* buffer, int* read_i) {
    return buffer[*read_i] == 0; // if 1, queue is empty
}

void printBuffer(int* buffer, int* read_i) {
    int i = *read_i;
    while (i < BUFFER_SIZE + (*read_i)) {
        // printf("(%d): ", i % BUFFER_SIZE);
        if (buffer[i % BUFFER_SIZE] == 0) {
			printf("_ ");
		} else {
			printf("%d ", buffer[i % BUFFER_SIZE]);
		}
        i++;
    }
	printf("\n");
}

void push(int* buffer, int n, int* write_i)
{
    buffer[*write_i] = n;
    *write_i = ((*write_i) + 1) % BUFFER_SIZE;
}

int pop(int* buffer, int* read_i)
{
    int num = buffer[*read_i];
    buffer[*read_i] = 0;
    *read_i = ((*read_i) + 1) % BUFFER_SIZE;

    return num;
}

void initialise_buffer(int* buffer, int* read_i) {
    printf("Initialising shared memory buffer: ");
    for (int i = 0; i < BUFFER_SIZE; i++) {
        buffer[i] = 0;
    }
    printf("\n");
    printBuffer(buffer, read_i);
}

int main(int argc, char* argv[])
{
    consumer_sum = 0;

    /*      loop variables          */
    int i;
    /*      shared memory keys      */
    key_t shmkey;
    key_t shmkey2;
    key_t shmkey3;
    /*      shared memory ids        */
    int shmid;
    int shmid_read;
    int shmid_write;
    /*      synch semaphore         */ /*shared */
    sem_t* sem;
    /*      fork pid                */
    pid_t pid;
    /*      shared variable         */ /*shared */
    int* p;
    int* read_i;
    int* write_i;
    /*      fork count              */
    unsigned int n;
    /*      semaphore value         */
    unsigned int value;
    /*      process label           */
    int processType = 0; // if processType < PRODUCERS -> producer else consumer

    /* initialize a shared variable in shared memory */
    shmkey = ftok("/dev/null", 5); /* valid directory name and a number */
    shmkey2 = ftok("/dev/null", 0); /* valid directory name and a number */
    shmkey3 = ftok("/dev/null", 1); /* valid directory name and a number */
    printf("shmkey for p = %d\n", shmkey);

    // maps file to 
    shmid = shmget(shmkey, sizeof(producer_buffer) * (BUFFER_SIZE + 4), 0644 | IPC_CREAT);
    shmid_read = shmget(shmkey2, sizeof(int), 0644 | IPC_CREAT);
    shmid_write = shmget(shmkey3, sizeof(int), 0644 | IPC_CREAT);

    if (shmid < 0 || shmid_read < 0 || shmid_write < 0) { /* shared memory error check */
        perror("shmget\n");
        exit(1);
    }

    p = (int*)shmat(shmid, NULL, 0); /* attach p to shared memory */
    read_i = (int*)shmat(shmid_read, NULL, 0); /* attach read_i to shared memory */
    write_i = (int*)shmat(shmid_write, NULL, 0); /* attach write_i to shared memory */

    // clear shared memory
    *read_i = 0;
    *write_i = 0;
    initialise_buffer(p, read_i);
    // printf("p=%d is allocated in shared memory.\n\n", *p);

    /********************************************************/

    /* initialize semaphores for shared processes */
    sem = sem_open("pSem", O_CREAT | O_EXCL, 0644, 1);
    /* name of semaphore is "pSem", semaphore is reached using this name */

    for (i = 0; i < PRODUCERS + CONSUMERS; i++) {
        pid = fork();
        if (pid < 0) {
            /* check for error      */
            sem_unlink("pSem");
            sem_close(sem);
            /* unlink prevents the semaphore existing forever */
            /* if a crash occurs during the execution         */
            printf("Fork error.\n");
        } else if (pid == 0) {
            processType = i;
            printf("Process type: %d\n", processType);
            break; /* child processes */
        } 
    }

    if (pid == 0 && processType < PRODUCERS) {
        while(1) {
            sem_wait(sem);
            // Add rand number to buffer here
            if (isQueueFull(p, write_i)) {
                printf("Producer(%d): Buffer is full!\n", i);
            } else {
                int num = (rand() % 10) + 1;
                printf("Producer(%d) writing %d to buffer[%d]: ", i, num, *write_i);
                push(p, num, write_i);
                printBuffer(p, read_i);
            }
            sem_post(sem);
            sleep(1);
        }
        exit(0);
    } else if (pid == 0 && processType >= PRODUCERS){
        while(1) {
            sem_wait(sem);
            if (isQueueEmpty(p, read_i)) {
                printf("Consumer: Buffer is empty!\n");
            } else {
                int index = *read_i;
                int num = pop(p, read_i);
                printf("Consumer (main process) reading %d from buffer[%d]: ", num, index);
                // need to share consumer_sum too
                consumer_sum += num;
                printBuffer(p, read_i);
                printf("Current consumer_sum: %d\n", consumer_sum);
            }
            sem_post(sem);
            sleep(1);
        }
        exit(0);
    } else {
        /* wait for all children to exit */
        while (pid == waitpid(-1, NULL, 0)) {
            if (errno == ECHILD)
                break;
        }
        printf("\nParent: All children have exited.\n");
        /* shared memory detach - cleanup allocated memory */
        shmdt(p);
        shmctl(shmid, IPC_RMID, 0);

        shmdt(read_i);
        shmctl(shmid_read, IPC_RMID, 0);

        shmdt(write_i);
        shmctl(shmid_write, IPC_RMID, 0);

        /* cleanup semaphores */
        sem_unlink("pSem");
        sem_close(sem);
        /* unlink prevents the semaphore existing forever */
        /* if a crash occurs during the execution         */
        exit(0);
    }

    printf("### consumer_sum final value = %d ###\n",
        consumer_sum);
    pthread_exit(NULL);
}




