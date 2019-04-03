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

void generateArray(int *array, unsigned int n)
{
	const int field = 10000;
	const int bias = field / 2;
	srand(time(NULL));
	for (unsigned int i = 0; i < n; i++)
		array[i] = (rand() % field) - bias;
}

void routine(int *arr, unsigned int begin, unsigned int end)
{
	printf("[%d; %d)\n", begin, end);
	for (unsigned int i = begin; i < end - 1 + begin; i++) {
		for (unsigned int j = begin; j < end - i - 1 + begin; j++) {
			if (arr[j] > arr[j + 1]) {
				int tmp = arr[j];
				arr[j] = arr[j + 1];
				arr[j + 1] = tmp;
			}
		}
	}
}

void merge(int *array, unsigned int n, int count)
{
	int *arr_new = malloc(sizeof(int) * n);
	if (!arr_new) {
		perror("Failed to allocate memory.");
		exit(1);
	}
	unsigned int size = 0, i = 0, j = 0;
	unsigned int left = n / count, right = left + !(n % 2 == 0);
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

void sort(int *array, int n)
{
	const unsigned int CORE_COUNT = 2;
	pid_t id[CORE_COUNT];
	unsigned int begins[CORE_COUNT], ends[CORE_COUNT];
	unsigned int step = n / CORE_COUNT;
	
	for (int i = 0; i < CORE_COUNT; i++) {
		begins[i] = step * i;
		if (i != CORE_COUNT - 1)
			ends[i] = step * (i + 1);
		else
			ends[i] = n;
	}
	for (int i = 0; i < CORE_COUNT; i++) {
		if ((id[i] = fork()) == 0) {
			cpu_set_t mask;
			CPU_ZERO(&mask);
			CPU_SET(i % CORE_COUNT, &mask);
			if (sched_setaffinity(0, sizeof(mask), &mask) == -1) {
				printf("WARNING: Could not set CPU Affinity, continuing.");
			}
			printf("SON: %d start.\n", getpid());
			routine(array, begins[i], ends[i]);
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
	merge(array, n, CORE_COUNT);
}

void print(int *array, int n)
{
	for (int i = 0; i < n; i++)
		printf("%d ", array[i]);
	putchar('\n');
}

int main(int argc, char *argv[])
{
	if (argc < 2) {
		puts("Too few arguments.");
		return 1;
	}
	unsigned int n = atoi(argv[1]);
	key_t key = ftok("/bin/bash", 'a');
	int shm_id = shmget(key, sizeof(int) * n, IPC_CREAT|IPC_EXCL|0600);
	int *array = shmat(shm_id, NULL, 0);
	if (array == (int *) -1) {
		perror("Failed to join shared memory.");
		return 1;
	}
	generateArray(array, n);
	//print(array, n);
	sort(array, n);
	//print(array, n);
	shmdt(array);
	shmctl(shm_id, IPC_RMID, 0);
	puts("Done.");
	return 0;
}
