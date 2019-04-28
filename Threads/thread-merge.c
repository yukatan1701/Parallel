#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>

struct arg_info {
	int n;
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
	int *array = malloc(sizeof(int) * n);
	if (!array) {
		perror("Failed to allocate memory.\n");
		exit(0);
	}
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
	int len = info->to - info->from + 1;
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
	int *arr_new = malloc(sizeof(int) * n);
	if (!arr_new) {
		perror("Failed to allocate memory.");
		exit(1);
	}
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
	for (int i = 0 ; i < n; i++)
		array[i] = arr_new[i];
	//memcpy(array, arr_new, n);
	free(arr_new);
}

void makeFromTo(int *array, const unsigned int count,
	unsigned long n, struct arg_info info[])
{
	unsigned long step = n / count;
	for (int i = 0; i < count; i++) {
		info[i].from = i * step;
		if (i == count - 1)
			info[i].to = n;
		else
			info[i].to = (i + 1) * step;
		info[i].array = array;
		info[i].n = n;
	}
}

void sort(int *array, unsigned long n)
{
	const unsigned int CORE_COUNT = 2;
	struct arg_info info[CORE_COUNT];
	//unsigned long from[CORE_COUNT], to[CORE_COUNT];
	pthread_t id[CORE_COUNT];
	makeFromTo(array, CORE_COUNT, n, info);
	for (int i = 0; i < CORE_COUNT; i++) {
		if (pthread_create(&id[i], NULL, &routine, &info[i]) == 0) {
			printf("Thread %d created.\n", i);
		}
	}
	for (int i = 0; i < CORE_COUNT; i++) {
		pthread_join(id[i], NULL);
		printf("Thread %d ended.\n", i);
	}
	merge(array, n, CORE_COUNT);
}

int main(int argc, char *argv[])
{
	if (argc < 2) {
		puts("Too few arguments.");
		return 1;
	}
	unsigned long n = atol(argv[1]);
	int *array = generateArray(n);
	print(array, n);
	sort(array, n);
	print(array, n);
	puts("Done.");
	return 0;
}
