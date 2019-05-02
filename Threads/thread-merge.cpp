#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <unistd.h>
#include <pthread.h>
#include <cstring>
#include <iostream>

using std::cout;
using std::endl;

struct arg_info {
	unsigned long n;
	int *array;
	unsigned long from, to;
};

void print(int *array, unsigned long n)
{
	for (unsigned long i = 0; i < n; i++)
		printf("%d ", array[i]);
	putchar('\n');
}

int *generateArray(unsigned long n)
{
	int *array = new int[n];
	const int field = 10000;
	const int bias = field / 2;
	srand(time(NULL));
	for (unsigned long i = 0; i < n; i++)
		array[i] = (rand() % field) - bias;
	return array;
}

void *routine(void *arg)
{
	struct arg_info *info = (struct arg_info *) arg;
	int *a = info->array + info->from;
	unsigned long len = info->to - info->from + 1;
	for (unsigned long i = 0; i < len - 1; i++) {
		for (unsigned long j = 0; j < len - i - 1; j++) {
			if (a[j] > a[j + 1]) {
				int tmp = a[j];
				a[j] = a[j + 1];
				a[j + 1] = tmp;
			}
		}
	}
	pthread_exit(NULL);
}

void merge(int *array, unsigned long n, int count)
{
	int *arr_new = new int[n];
	unsigned long size = 0, i = 0, j = 0;
	unsigned long left = n / count, right = left + !(n % 2 == 0);
	int *a = array, *b = array + left;
	while (size != n) {
		if (i == left && j < right) {
			arr_new[size++] = b[j++];
			continue;
		}
		if (j == right && i < left) {
			arr_new[size++] = a[i++];
			continue;
		}
		if (a[i] < b[j])
			arr_new[size] = a[i++];
		else if (a[i] >= b[j])
			arr_new[size] = b[j++];
		size++;
	}
	for (unsigned long i = 0 ; i < n; i++)
		array[i] = arr_new[i];
	//memcpy(array, arr_new, n);
	delete[] arr_new;
}

void makeFromTo(int *array, const unsigned int count,
	unsigned long n, struct arg_info info[])
{
	unsigned long step = n / count;
	for (unsigned long i = 0; i < count; i++) {
		info[i].from = i * step;
		if (i == count - 1)
			info[i].to = n;
		else
			info[i].to = (i + 1) * step;
		info[i].array = array;
		info[i].n = n;
	}
}

void sort(int *array, unsigned long n, bool debug)
{
	const int CORE_COUNT = 2;
	struct arg_info info[CORE_COUNT];
	//unsigned long from[CORE_COUNT], to[CORE_COUNT];
	pthread_t id[CORE_COUNT];
	makeFromTo(array, CORE_COUNT, n, info);
	for (int i = 0; i < CORE_COUNT; i++) {
		if (pthread_create(&id[i], NULL, &routine, &info[i]) == 0) {
			if (debug)
				printf("Thread %d created.\n", i);
		}
	}
	for (int i = 0; i < CORE_COUNT; i++) {
		pthread_join(id[i], NULL);
		if (debug)
			printf("Thread %d ended.\n", i);
	}
	merge(array, n, CORE_COUNT);
}

void parseArgs(unsigned long & n, bool & debug, int argc, char *argv[])
{
	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-n") == 0) {
			if (i + 1 < argc)
				n = atol(argv[i + 1]);
		} else if (strcmp(argv[i], "-d") == 0) {
			debug = true;
		}
	}
}

int main(int argc, char *argv[])
{
	unsigned long n = 10;
	bool debug = false;
	parseArgs(n, debug, argc, argv);
	int *array = generateArray(n);
	if (debug)
		print(array, n);
	sort(array, n, debug);
	if (debug)
		print(array, n);
	cout << "Array size: " << n << endl;
	cout << "Done." << endl;
	delete[] array;
	return 0;
}
