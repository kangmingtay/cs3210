#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
	/* Accepts filepaths to two input DNA sequences as command-line arguments*/
	if (argc != 3) {
		printf("Usage: [executable] [file 1] [file 2]\n");
		return 1;
	}

	/* Validate both filepaths*/
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

	/* Initialise two rows of the matrix with (M + 1) rows x (N + 1) columns*/
	int *mat[2];
	mat[0] = calloc(N + 1, sizeof(int));
	mat[1] = malloc((N + 1) * sizeof(int));
	mat[1][0] = 0;

	int old = 0;
	int new = 1;
	for (int i = 1; i <= M; i++) {
		for (int j = 1; j <= N; j++) {
			if (seq1[i - 1] == seq2[j - 1]) {
				mat[new][j] = mat[old][j - 1] + 1;
			} else {
				int left = mat[new][j - 1];
				int down = mat[old][j];
				mat[new][j] = left > down ? left : down;
			}
		}
		/* Swap the previous and current row*/
		old = old ^ new;
		new = new ^ old;
		old = old ^ new;
	}

	printf("%d", mat[old][N]);

	free(seq1);
	free(seq2);
	free(mat[0]);
	free(mat[1]);
};

