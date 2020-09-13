#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>

#define NUMBER_OF_THREADS 5

int* mat[3];
int D_old = 0;
int D_prev = 1;
int D_current = 2;
int longer_edge, shorter_edge;
char * longer_seq;
char * shorter_seq;
bool done = false;

struct work_struct {
	int di1;
	int di2;
	int d;
	bool increasing;
};

struct work_struct work_allocation[NUMBER_OF_THREADS];

pthread_barrier_t barrier_work_finished;
pthread_barrier_t barrier_work_allocated;

void increase(int di1, int di2, int d) {
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

void decrease(int di1, int di2, int d) {
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

void * worker (void *tid) {
	long threadid = (long)tid;
	while(!done) {
		pthread_barrier_wait (&barrier_work_allocated); /*wait for main to assign work then signal start*/

		/*read work allocation/check if there is work to do*/
		if (work_allocation[threadid].di2 != 0) {
			int di1 = work_allocation[threadid].di1;
			int di2 = work_allocation[threadid].di2;
			work_allocation[threadid].di2 = 0;
			int d = work_allocation[threadid].d;
			bool increasing = work_allocation[threadid].increasing;

			/*do work*/
			if (increasing) {
				increase(di1, di2, d);
			} else {
				decrease(di1, di2, d);
			}
		}
		pthread_barrier_wait (&barrier_work_finished); /*signals main that work has been done*/
	}
}

int main(int argc, char *argv[]) {
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

	mat[D_old] = calloc(1, sizeof(int));
	mat[D_prev] = calloc(2, sizeof(int));
	mat[D_current] = calloc(2, sizeof(int));

	pthread_barrier_init (&barrier_work_allocated, NULL, NUMBER_OF_THREADS + 1);
	pthread_barrier_init (&barrier_work_finished, NULL, NUMBER_OF_THREADS + 1);

	pthread_t worker_threads[NUMBER_OF_THREADS];
	for (long tid = 0; tid < NUMBER_OF_THREADS; tid++) {
		int rc = pthread_create(&worker_threads[tid], NULL, worker, (void*)tid);
		if (rc) {
			printf("Error: Return code from pthread_create() is %d\n", rc);
			exit(-1);
		}
		work_allocation[tid] = (struct work_struct){0,0,0,0};
	}

	int d = 2;
	bool is_increasing = true;
	while (d >= 0) {
		free(mat[D_current]);
		int length_of_diagonal = (d > shorter_edge ? shorter_edge : d) + 1;
		mat[D_current] = malloc(length_of_diagonal * sizeof(int));
		int smaller_work = length_of_diagonal / NUMBER_OF_THREADS;
		int larger_work = smaller_work + 1;
		int tid = 0;
		int di; /*index of element on diagonal*/
		int total_larger_work =  (length_of_diagonal % NUMBER_OF_THREADS) * larger_work;
		for (di = 0; di < total_larger_work; di += larger_work) {
			work_allocation[tid++] = (struct work_struct){di, di + larger_work, d, is_increasing};    
		}
		for (; di < length_of_diagonal; di += smaller_work) {
			work_allocation[tid++] = (struct work_struct){di, di + smaller_work, d, is_increasing};    
		}

		pthread_barrier_wait(&barrier_work_allocated); /*signal worker, work is allocated*/
		pthread_barrier_destroy(&barrier_work_allocated);
		pthread_barrier_init(&barrier_work_allocated, NULL, NUMBER_OF_THREADS + 1); /*rest &barrier_work_allocated*/
		pthread_barrier_wait(&barrier_work_finished); /*If workers have finished their work, continue code execution*/
		pthread_barrier_destroy(&barrier_work_finished);
		pthread_barrier_init(&barrier_work_finished, NULL, NUMBER_OF_THREADS + 1); /*rest &barrier_work_finished*/

		D_old = D_prev;
		D_prev = D_current;
		D_current = 3 - D_prev - D_old;

		if (is_increasing) d++;
		else d --;

		if (d > longer_edge) {
			d = shorter_edge - 1;
			is_increasing = false;
		}
	}

	pthread_barrier_wait(&barrier_work_allocated);
	done = true;
	pthread_barrier_wait(&barrier_work_finished);

	for (int tid = 0; tid < NUMBER_OF_THREADS; tid++)
	{
		pthread_join(worker_threads[tid], NULL);
	}

	printf("%d",mat[D_prev][0]);

	free(mat[0]);
	free(mat[1]);
	free(mat[2]);
	free(seq1);
	free(seq2);
	pthread_exit(NULL);
}
