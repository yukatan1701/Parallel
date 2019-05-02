#include <cstdio>
#include <pthread.h>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <unistd.h>
#include <cstring>

using std::cout;
using std::endl;

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
	double **mx = new double*[n];
	const int field = 10;
	const int bias = field / 2;
	srand(time(NULL));
	for (unsigned long i = 0; i < n; i++) {
		mx[i] = new double[n];
		double sum = 0;
		for (unsigned long j = 0; j < n; j++) {
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

void run_threads(double **mx, int k, int n, int count, bool debug) {
	const unsigned int CORE_COUNT = count;
	struct arg_info info[CORE_COUNT];
	pthread_t id[CORE_COUNT];
	int cur_count = makeFromTo(mx, k, n, CORE_COUNT, info);
	for (int i = 0; i < cur_count; i++) {
		if (pthread_create(&id[i], NULL, &routine, &info[i]) == 0) {
			if (debug)
				printf("Thread %d created.\n", i);
		}
	}
	for (int i = 0; i < cur_count; i++) {
		pthread_join(id[i], NULL);
		if (debug)
			printf("Thread %d ended.\n", i);
	}
}

void gauss(double **mx, int n, int count, bool debug)
{	

	for (int k = 0; k < n; k++) {
		for (int j = k + 1; j < n; j++)
			mx[k][j] = mx[k][j] / mx[k][k];
		run_threads(mx, k, n, count, debug);
	}	
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
	double **matrix = generateMatrix(n);
	if (debug)
		print(matrix, n);
	gauss(matrix, n, count, debug);
	if (debug)
		print(matrix, n);
	cout << "Count of cores: " << count << endl;
	cout << "Matrix size: " << n << endl;
	cout << "\nDone." << endl;
	for (unsigned long i = 0; i < n; i++)
		delete[] matrix[i];
	delete[] matrix;
	return 0;
}
