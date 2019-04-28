#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sched.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <wait.h>


struct arg_info {
	int k, n;
	int from, to;
};

static double *plain;

double **line_to_matrix(double *plain, int n)
{
	double **mx = malloc(sizeof(double *) * n);
	if (!mx) {
		perror("Failed to allocate memory.");
		exit(0);
	}
	for (int i = 0; i < n; i++)
		mx[i] = plain + i * n;
	return mx;
}

void print(unsigned long n)
{
	for (unsigned long i = 0; i < n; i++) {
		for (unsigned long j = 0; j < n; j++)
			printf("%5.2lf ", (plain + i * n)[j]);
		putchar('\n');
	}
	putchar('\n');
}

void generateMatrix(unsigned long n)
{
	const int field = 10;
	const int bias = field / 2;
	srand(time(NULL));
	for (int i = 0; i < n; i++) {
		double sum = 0;
		for (int j = 0; j < n; j++) {
			(plain + i * n)[j] = (rand() % field) - bias;
			sum += abs((plain + i * n)[j]);
		}
		(plain + i * n)[i] = abs((plain + i * n)[i]) + sum;
	}
}

void routine(struct arg_info info)
{
	int n = info.n, k = info.k;
	double **mx = line_to_matrix(plain, n);
	int from = info.from, to = info.to;
	//printf("From: %d, to: %d, n: %d, k: %d\n", from, to, n, k);
	for (int i = from; i < to; i++) {
		for (int j = k + 1; j < n; j++) {
			mx[i][j] -= mx[i][k] * mx[k][j];
		}
		mx[i][k] = 0;
	}
	free(mx);
}

int makeFromTo(int k, int n, int count, struct arg_info info[])
{
	while (n - k < count)
		count--;
	int step = (n - k) / count;
	int bias = k + 1;
	for (int i = 0; i < count; i++) {
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

void run_forks(double **mx, int k, int n) {
	const unsigned int CORE_COUNT = 4;
	struct arg_info info[CORE_COUNT];
	pid_t id[CORE_COUNT];
	int cur_count = makeFromTo(k, n, CORE_COUNT, info);
	for (int i = 0; i < cur_count; i++) {
		if ((id[i] = fork()) == 0) {
			cpu_set_t mask;
			CPU_ZERO(&mask);
			CPU_SET(i % CORE_COUNT, &mask);
			if (sched_setaffinity(0, sizeof(mask), &mask) == -1) {
				printf("WARNING: Could not set CPU Affinity, continuing.");
			}
			printf("SON: %d start.\n", getpid());
			routine(info[i]);
			printf("SON: %d ended.\n", getpid());
			exit(0);
		}
		printf("PARENT: %d start\n", id[i]);
	}
	pid_t wpid;
	int status;
	while ((wpid = wait(&status)) > 0)
		;
	puts("Children are over.");
}

void gauss(int n)
{
	double **mx = line_to_matrix(plain, n);
	for (int k = 0; k < n; k++) {
		for (int j = k + 1; j < n; j++) {
			mx[k][j] = mx[k][j] / mx[k][k];
		}
		//printf("--- k = %d ---\n", k);
		//print(n);
		run_forks(mx, k, n);
	}
	free(mx);
}

int main(int argc, char *argv[])
{
	if (argc < 2) {
		puts("Too few arguments.");
		return 1;
	}
	unsigned long n = atol(argv[1]);
	int shm_id = shmget(IPC_PRIVATE, sizeof(double) * (int) (n * n), IPC_CREAT|0666);
	plain = shmat(shm_id, NULL, 0);
	if (plain == (void *) -1) {
		perror("Failed to join shared memory.");
		return 1;
	}
	generateMatrix(n);
	print(n);
	gauss(n);
	putchar('\n');
	print(n);
	shmdt(plain);
	shmctl(shm_id, IPC_RMID, 0);
	puts("Done.");
	return 0;
}
