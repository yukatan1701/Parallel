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

void generateArray(int *array, unsigned long n)
{
	const int field = 10000;
	const int bias = field / 2;
	srand(time(NULL));
	for (unsigned long i = 0; i < n; i++)
		array[i] = (rand() % field) - bias;
}

void routine(int *arr, unsigned long begin, unsigned long end)
{
	//printf("[%lu; %lu)\n", begin, end);
	for (unsigned long i = begin; i < end - 1 + begin; i++) {
		for (unsigned long j = begin; j < end - i - 1 + begin; j++) {
			if (arr[j] > arr[j + 1]) {
				int tmp = arr[j];
				arr[j] = arr[j + 1];
				arr[j + 1] = tmp;
			}
		}
	}
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

void sort(int *array, unsigned long n, bool debug)
{
	const unsigned long CORE_COUNT = 2;
	pid_t id[CORE_COUNT];
	unsigned long begins[CORE_COUNT], ends[CORE_COUNT];
	unsigned long step = n / CORE_COUNT;
	
	for (unsigned long i = 0; i < CORE_COUNT; i++) {
		begins[i] = step * i;
		if (i != CORE_COUNT - 1)
			ends[i] = step * (i + 1);
		else
			ends[i] = n;
	}
	for (unsigned long i = 0; i < CORE_COUNT; i++) {
		if ((id[i] = fork()) == 0) {
			cpu_set_t mask;
			CPU_ZERO(&mask);
			CPU_SET(i % CORE_COUNT, &mask);
			if (sched_setaffinity(0, sizeof(mask), &mask) == -1) {
				printf("WARNING: Could not set CPU Affinity, continuing.");
			}
			if (debug)
				printf("SON: %d start.\n", getpid());
			routine(array, begins[i], ends[i]);
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
	merge(array, n, CORE_COUNT);
}

void print(int *array, unsigned long n)
{
	for (unsigned long i = 0; i < n; i++)
		printf("%d ", array[i]);
	putchar('\n');
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
	int shm_id = shmget(IPC_PRIVATE, sizeof(int) * n, IPC_CREAT|0666);
	int *array = (int *) shmat(shm_id, NULL, 0);
	if (array == (int *) -1) {
		perror("Failed to join shared memory.");
		return 1;
	}
	generateArray(array, n);
	if (debug)
		print(array, n);
	sort(array, n, debug);
	if (debug)
		print(array, n);
	shmdt(array);
	shmctl(shm_id, IPC_RMID, 0);
	cout << "Array size: " << n << endl;
	cout << "Done." << endl;
	return 0;
}
