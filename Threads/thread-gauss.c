#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <time.h>

struct arg_info {
	int k, n;
	double **matrix;
	int from, to;
};

void print(double **matrix, unsigned long n)
{
	for (unsigned long i = 0; i < n; i++) {
		for (unsigned long j = 0; j < n; j++)
			printf("%5.2lf ", matrix[i][j]);
		putchar('\n');
	}
	putchar('\n');
}

double **generateMatrix(unsigned long n)
{
	double **mx = malloc(sizeof(double *) * n);
	if (!mx) {
		perror("Failed to allocate memory.\n");
		exit(0);
	}
	const int field = 10;
	const int bias = field / 2;
	srand(time(NULL));
	for (int i = 0; i < n; i++) {
		mx[i] = malloc(sizeof(double) * n);
		if (!mx[i]) {
			perror("Failed to allocate memory.\n");
			exit(0);
		}
		double sum = 0;
		for (int j = 0; j < n; j++) {
			mx[i][j] = (rand() % field) - bias;
			sum += abs(mx[i][j]);
		}
		mx[i][i] = abs(mx[i][i]) + sum;
	}
	return mx;
}

void *routine(void *arg)
{
	struct arg_info *info = (struct arg_info *) arg;
	double **mx = info->matrix;
	int n = info->n, k = info->k;
	int from = info->from, to = info->to;
	for (int i = from; i < to; i++) {
		for (int j = k + 1; j < n; j++)
			mx[i][j] = mx[i][j] - mx[i][k] * mx[k][j];
		mx[i][k] = 0;
	}
	pthread_exit(NULL);
}

int makeFromTo(double **mx, int k, int n, int count, struct arg_info info[])
{
	while (n - k < count)
		count--;
	int step = (n - k) / count;
	int bias = k + 1;
	for (int i = 0; i < count; i++) {
		info[i].matrix = mx;
		info[i].k = k;
		info[i].n = n;
		info[i].from = bias + i * step;
		if (i == count - 1)
			info[i].to = n;
		else
			info[i].to = bias + (i + 1) * step;

	}
	return count;
}

void run_threads(double **mx, int k, int n) {
	const unsigned int CORE_COUNT = 4;
	struct arg_info info[CORE_COUNT];
	pthread_t id[CORE_COUNT];
	int cur_count = makeFromTo(mx, k, n, CORE_COUNT, info);
	for (int i = 0; i < cur_count; i++) {
		if (pthread_create(&id[i], NULL, &routine, &info[i]) == 0) {
			//printf("Thread %d created.\n", i);
		}
	}
	for (int i = 0; i < cur_count; i++) {
		pthread_join(id[i], NULL);
		//printf("Thread %d ended.\n", i);
	}
}

void gauss(double **mx, int n)
{	

	for (int k = 0; k < n; k++) {
		for (int j = k + 1; j < n; j++)
			mx[k][j] = mx[k][j] / mx[k][k];
		run_threads(mx, k, n);
	}	
}

int main(int argc, char *argv[])
{
	if (argc < 2) {
		puts("Too few arguments.");
		return 1;
	}
	unsigned long n = atol(argv[1]);
	double **matrix = generateMatrix(n);
	print(matrix, n);
	gauss(matrix, n);
	print(matrix, n);
	puts("Done.");
	return 0;
}
