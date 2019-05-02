#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <unistd.h>
#include <sched.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <wait.h>
#include <iostream>

using std::cout;
using std::endl;

struct arg_info {
	int k, n;
	int from, to;
};

static double *plain;

double **line_to_matrix(double *plain, int n)
{
	double **mx = new double*[n];
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
	for (unsigned long i = 0; i < n; i++) {
		double sum = 0;
		for (unsigned long j = 0; j < n; j++) {
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
	delete[] mx;
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

void run_forks(double **mx, int k, int n, int count, bool debug) {
	const unsigned int CORE_COUNT = count;
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
			if (debug)
				printf("SON: %d start.\n", getpid());
			routine(info[i]);
			if (debug)
				printf("SON: %d ended.\n", getpid());
			exit(0);
		}
		if (debug)
			printf("PARENT: %d start\n", id[i]);
	}
	pid_t wpid;
	int status;
	while ((wpid = wait(&status)) > 0)
		;
	if (debug)
		puts("Children are over.");
}

void gauss(int n, int count, bool debug)
{
	double **mx = line_to_matrix(plain, n);
	for (int k = 0; k < n; k++) {
		for (int j = k + 1; j < n; j++) {
			mx[k][j] = mx[k][j] / mx[k][k];
		}
		//printf("--- k = %d ---\n", k);
		//print(n);
		run_forks(mx, k, n, count, debug);
	}
	free(mx);
}

void parseArgs(unsigned long & n, unsigned long & count, bool & debug,
	int argc, char *argv[])
{
	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-n") == 0) {
			if (i + 1 < argc)
				n = atol(argv[i + 1]);
		} else if (strcmp(argv[i], "-c") == 0) {
			if (i + 1 < argc)
				count = atol(argv[i + 1]);
		} else if (strcmp(argv[i], "-d") == 0) {
			debug = true;
		}
	}
}

int main(int argc, char *argv[])
{
	unsigned long n = 5;
	unsigned long sys_core_count = sysconf(_SC_NPROCESSORS_ONLN); 
	unsigned long count = sys_core_count;
	bool debug = false;
	parseArgs(n, count, debug, argc, argv);
	if (count > sys_core_count) {
		cout << "WARNING: count of cores is too large. Use system count: "
			<< sys_core_count << "." << endl;
	}
	if (count < 1) {
		cout << "WARNING: count of cores is too small. Use minimum count: 1."
			<< endl;
	}
	int shm_id = shmget(IPC_PRIVATE, sizeof(double) * (int) (n * n), IPC_CREAT|0666);
	plain = (double *) shmat(shm_id, NULL, 0);
	if (plain == (void *) -1) {
		perror("Failed to join shared memory.");
		return 1;
	}
	generateMatrix(n);
	if (debug)
		print(n);
	gauss(n, count, debug);
	putchar('\n');
	if (debug)
		print(n);
	shmdt(plain);
	shmctl(shm_id, IPC_RMID, 0);
	cout << "Count of cores: " << count << endl;
	cout << "Matrix size: " << n << endl;
	cout << "\nDone." << endl;
	return 0;
}
