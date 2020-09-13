#include <errno.h>
#include <fcntl.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>

#define NUMBER_OF_PROCESSES 20

int * mat[3];
int longer_edge, shorter_edge;
char * longer_seq;
char * shorter_seq;

void increase(int di1, int di2, int d, int D_current, int D_prev, int D_old) {
	for (int di = di1; di < di2; di++) {
		if ((d - di - 1 < 0) || (di - 1 < 0)) {
			mat[D_current][di] = 0;
		}
		else if (longer_seq[d - di - 1] == shorter_seq[di - 1]) { 
			mat[D_current][di] = mat[D_old][di - 1] + 1; 
		} else {
			int left = mat[D_prev][di];
			int down = mat[D_prev][di - 1];
			mat[D_current][di] = left > down ? left : down;
		}
	}
}

void decrease(int di1, int di2, int d, int D_current, int D_prev, int D_old) {
	for (int di = di1; di < di2; di ++) {
		if (longer_seq[longer_edge - di - 1] == shorter_seq[shorter_edge - d + di - 1]) {
			mat[D_current][di] = ((d == shorter_edge - 1) ? mat[D_old][di] : mat[D_old][di + 1]) + 1;
		} else {
			int left = mat[D_prev][di + 1];
			int down = mat[D_prev][di];
			mat[D_current][di] = left > down ? left : down;
		}
	}
}

typedef struct work_struct {
	int di1;
	int di2;
	int d;
	bool increasing;
}task;

typedef struct shared_memory {
	int D_old;
	int D_prev;
	int D_current;
	task work[NUMBER_OF_PROCESSES];
	bool done;
	pthread_barrier_t barrier_work_finished;
	pthread_barrier_t barrier_work_allocated;
}shared_mem;

int main(int argc, char** argv)
{
	if (argc != 3) {
		printf("Usage: [executable] [file 1] [file 2]\n");
		return 1;
	}

	FILE *file1 = fopen(argv[1], "r");
	FILE *file2 = fopen(argv[2], "r");
	if (!file1 || !file2) {
		printf("Input files are not found!\n");
		return 1;
	}

	int M, N;
	fscanf(file1, "%d", &M);
	fscanf(file2, "%d", &N);
	char *seq1 = (char*) malloc(M + 1);
	char *seq2 = (char*) malloc(N + 1);
	fscanf(file1, "%s", seq1);
	fscanf(file2, "%s", seq2);
	fclose(file1);
	fclose(file2);

	if (M<N) {
		longer_edge = N;
		shorter_edge = M;
		longer_seq = seq2;
		shorter_seq = seq1;
	} else {
		longer_edge = M;
		shorter_edge = N;
		longer_seq = seq1;
		shorter_seq = seq2;
	}

	int process_num;
	key_t shmkey;
	int shmid;
	pid_t pid;
	int* p;

	shmkey = ftok("/dev/null", 6);
	shmid = shmget(shmkey, sizeof(shared_mem) + (shorter_edge + 1) * 3 * sizeof(int), 0644 | IPC_CREAT);
	if (shmid < 0) { /* shared memory error check */
		perror("shmget\n");
		exit(1);
	}

	p = (int*)shmat(shmid, NULL, 0); /* attach p to shared memory */
	shared_mem * sm = (shared_mem *)(p + (shorter_edge + 1) * 3);
	mat[0] = p;
	mat[1] = mat[0] + shorter_edge + 1;
	mat[2] = mat[1] + shorter_edge + 1;
	sm->D_current = 2;
	sm->D_prev = 1;
	sm->D_old = 0;
	sm->done = false;

	pthread_barrierattr_t attr;
	pthread_barrierattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);

	pthread_barrier_init (&(sm->barrier_work_allocated), &attr, NUMBER_OF_PROCESSES + 1);
	pthread_barrier_init (&(sm->barrier_work_finished), &attr, NUMBER_OF_PROCESSES + 1);    

	/********************************************************/

	/* fork child processes */
	for (process_num = 0; process_num < NUMBER_OF_PROCESSES; process_num++) {
		sm->work[process_num] = (task){0,0,0,true};
		pid = fork();
		if (pid < 0) {
			printf("Fork error.\n");
		} else if (pid == 0)
			break; /* child processes */
	}
	/******************************************************/
	/******************   CHILD PROCESS   *****************/
	/******************************************************/
	if (pid == 0) {
		while(!sm->done) {
			pthread_barrier_wait (&(sm->barrier_work_allocated)); /*wait for main to assign work then signal start*/

			/*read work allocation/check if there is work to do*/
			if (sm->work[process_num].di2 != 0) {
				/*do work*/
				if (sm->work[process_num].increasing) {
					increase(sm->work[process_num].di1, sm->work[process_num].di2, sm->work[process_num].d, sm->D_current, sm->D_prev, sm->D_old);
				} else {
					decrease(sm->work[process_num].di1, sm->work[process_num].di2, sm->work[process_num].d, sm->D_current, sm->D_prev, sm->D_old);
				}
				sm->work[process_num].di2 = 0;
			}
			pthread_barrier_wait (&(sm->barrier_work_finished)); /*signals main that work has been done*/
		}
		exit(0);
	}

	/******************************************************/
	/******************   PARENT PROCESS   ****************/
	/******************************************************/
	else {
		mat[sm->D_old][0] = 0;
		mat[sm->D_prev][0] = 0;
		mat[sm->D_prev][1] = 0;
		int d = 2;
		bool is_increasing = true;
		while (d >= 0) {
			int length_of_diagonal = (d > shorter_edge ? shorter_edge : d) + 1;
			int smaller_work = length_of_diagonal / NUMBER_OF_PROCESSES;
			int larger_work = smaller_work + 1;
			int tid = 0;
			int di; /*index of element on diagonal*/
			int total_larger_work =  (length_of_diagonal % NUMBER_OF_PROCESSES) * larger_work;
			for (di = 0; di < total_larger_work; di += larger_work) {
				sm->work[tid++] = (task){di, di + larger_work, d, is_increasing};    
			}
			for (; di < length_of_diagonal; di += smaller_work) {
				sm->work[tid++] = (task){di, di + smaller_work, d, is_increasing};    
			}
			pthread_barrier_wait(&(sm->barrier_work_allocated)); /*signal worker, work is allocated*/
			pthread_barrier_destroy(&(sm->barrier_work_allocated));
			pthread_barrier_init(&(sm->barrier_work_allocated), &attr, NUMBER_OF_PROCESSES + 1); /*rest &barrier_work_allocated*/
			pthread_barrier_wait(&(sm->barrier_work_finished)); /*If workers have finished their work, continue code execution*/
			pthread_barrier_destroy(&(sm->barrier_work_finished));
			pthread_barrier_init(&(sm->barrier_work_finished), &attr, NUMBER_OF_PROCESSES + 1); /*rest &barrier_work_finished*/

			sm->D_old = sm->D_prev;
			sm->D_prev = sm->D_current;
			sm->D_current = 3 - sm->D_prev - sm->D_old;

			if (is_increasing) d++;
			else d --;

			if (d > longer_edge) {
				d = shorter_edge - 1;
				is_increasing = false;
			}
		}

		pthread_barrier_wait(&(sm->barrier_work_allocated));
		sm->done = true;
		pthread_barrier_wait(&(sm->barrier_work_finished));
		/* wait for all children to exit */
		while (pid = waitpid(-1, NULL, 0)) {
			if (errno == ECHILD)
				break;
		}
		printf("%d",mat[sm->D_prev][0]);
		/* shared memory detach */
		shmdt(p);
		shmctl(shmid, IPC_RMID, 0);
	}
	exit(0);
}
