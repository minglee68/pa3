#define _GNU_SOURCE
#include <dlfcn.h>
#include <execinfo.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>

#define N 200

static pthread_mutex_t *graph[10][100];

static int lock[10][100];
static int try[100][10];

static pthread_mutex_t *mutex_index[100] = {NULL};
static unsigned int thread_index[10] = {0};


pthread_mutex_t lock_mutex1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock_mutex2 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock_mutex3 = PTHREAD_MUTEX_INITIALIZER;

static int n = 0;

int check_lock(int orig_thread, int mutex_flag);
int check_try(int orig_thread, int thread_flag);
int check_lock_a(int orig_mutex, int mutex_flag);
int check_try_a(int orig_mutex, int thread_flag);

int check_lock(int orig_thread, int mutex_flag) {
	int i = 0;
	for(i = 0; i < 10; i++) {
		if (lock[i][mutex_flag] == 1) {
			//fprintf(stderr, "there is lock on %p by %d\n", mutex_index[mutex_flag], thread_index[i]);
			fprintf(stderr, "->lock on m%d by t%d", mutex_flag, i);
			if (i == orig_thread)
				return -1;
			return check_try(orig_thread, i);
		}
	}

	return 0;
}

int check_try(int orig_thread, int thread_flag) {
	int i = 0;
	for (i = 0; i < 100; i++) {
		if (try[i][thread_flag] == 1) {
			//fprintf(stderr, "there is trial on %p by %d\n", mutex_index[i], thread_index[thread_flag]);
			fprintf(stderr, "->try by t%d on m%d", thread_flag, i);
			return check_lock(orig_thread, i);
		}
	}
	
	return 0;
}

int check_lock_a(int orig_mutex, int mutex_flag) {
	int i = 0;
	for(i = 0; i < 10; i++) {
		if (lock[i][mutex_flag] == 1) {
			//fprintf(stderr, "there is lock on %p by %d\n", mutex_index[mutex_flag], thread_index[i]);
			fprintf(stderr, "->lock on m%d by t%d", mutex_flag, i);
			return check_try_a(orig_mutex, i);
		}
	}

	return 0;
}

int check_try_a(int orig_mutex, int thread_flag) {
	int i = 0;
	for (i = 0; i < 100; i++) {
		if (try[i][thread_flag] == 1) {
			//fprintf(stderr, "there is trial on %p by %d\n", mutex_index[i], thread_index[thread_flag]);
			fprintf(stderr, "->try by t%d on m%d", thread_flag, i);
			if (i == orig_mutex)
				return -1;
			return check_lock_a(orig_mutex, i);
		}
	}
	
	return 0;
}

int pthread_mutex_lock(pthread_mutex_t *mutex) {

	int (*pthread_mutex_lockp)(pthread_mutex_t *mutex);
	int (*pthread_mutex_unlockp)(pthread_mutex_t *mutex);
	char * error;

	pthread_mutex_lockp = dlsym(RTLD_NEXT, "pthread_mutex_lock");
	if ((error = dlerror()) != 0x0)
		exit(1);

	pthread_mutex_unlockp = dlsym(RTLD_NEXT, "pthread_mutex_unlock");
	if ((error = dlerror()) != 0x0)
		exit(1);

	//fprintf(stderr, "lock trial by %d!!\n", (unsigned int)pthread_self());

/*
	int i = 0;
	int j = 0;
	int flag = 0;
	for (i = 0; i < 10; i++) {
		if (thread_index[i] == 0)
			break;
		for (j = 0; j < 100; j++) {
			if (graph[i][j] == NULL)
				break;
			if (graph[i][j] == mutex) {
				fprintf(stderr, "deadlock detected, %p, %p\n", graph[i][j], mutex);
				flag = 1;
			}
		}
	}
*/
	//fprintf(stderr, "lock called for %p by %d\n", mutex, (unsigned int)pthread_self());
	//pthread_mutex_lockp(&lock_mutex1);

	int i = 0;
	int j = 0;
	int thread_flag = -1;
	for (i = 0; i < 10; i++) {
		if (thread_index[i] == 0) {
			thread_index[i] = (unsigned int)pthread_self();
			thread_flag = i;
			for (j = 0; j < 100; j++) {
				lock[i][j] = 0;
			}
			break;
		}
		if (thread_index[i] == (unsigned int)pthread_self()){
			thread_flag = i;
			break;
		}
	}
	
	int mutex_flag = -1;
	for (i = 0; i < 100; i++) {
		if (mutex_index[i] == 0) {
			mutex_index[i] = mutex;
			mutex_flag = i;
			for (j = 0; j < 10; j++) {
				try[i][j] = 0;
			}
		}
		if (mutex_index[i] == mutex) {
			mutex_flag = i;
			break;
		}
	}

	try[mutex_flag][thread_flag] = 1;
	//fprintf(stderr, "trial to lock %p from %d\n", mutex_index[mutex_flag], thread_index[thread_flag]);

	fprintf(stderr, "try by t%d on m%d", thread_flag, mutex_flag);
	if (check_lock(thread_flag, mutex_flag) == -1)
		fprintf(stderr, "\nDeadlock Detected!\n");
	fprintf(stderr, "\n");

	//pthread_mutex_unlockp(&lock_mutex1);

	//pthread_mutex_lockp(&lock_mutex3);
	int temp = pthread_mutex_lockp(mutex);

	lock[thread_flag][mutex_flag] = 1;
	try[mutex_flag][thread_flag] = 0;

	fprintf(stderr, "lock on m%d by t%d", mutex_flag, thread_flag);
	if (check_try_a(mutex_flag, thread_flag) == -1)
		fprintf(stderr, "\nDeadlock Detected!\n");
	fprintf(stderr, "\n");

	//fprintf(stderr, "success to lock %p from %d\n", mutex_index[mutex_flag], thread_index[thread_flag]);

	//fprintf(stderr, "%d locks %p\n", (unsigned int)pthread_self(), mutex);

	//pthread_mutex_unlockp(&lock_mutex3);
	
	return temp;
}

int pthread_mutex_unlock(pthread_mutex_t *mutex) {

	int (*pthread_mutex_lockp)(pthread_mutex_t *mutex);
	int (*pthread_mutex_unlockp)(pthread_mutex_t *mutex);
	char * error;

	pthread_mutex_lockp = dlsym(RTLD_NEXT, "pthread_mutex_lock");
	if ((error = dlerror()) != 0x0)
		exit(1);

	pthread_mutex_unlockp = dlsym(RTLD_NEXT, "pthread_mutex_unlock");
	if ((error = dlerror()) != 0x0)
		exit(1);

	//fprintf(stderr, "unlock called for %p by %d\n", mutex, (unsigned int)pthread_self());

	//pthread_mutex_lockp(&lock_mutex2);

	int i = 0;
	int j = 0;
	int thread_flag = -1;
	for (i = 0; i < 10; i++) {
		if (thread_index[i] == (unsigned int)pthread_self()){
			thread_flag = i;
			break;
		}
	}
	
	int mutex_flag = -1;
	for (i = 0; i < 100; i++) {
		if (mutex_index[i] == mutex) {
			mutex_flag = i;
			break;
		}
	}

	int temp = pthread_mutex_unlockp(mutex);

	lock[thread_flag][mutex_flag] = 0;
	
	fprintf(stderr, "unlock on m%d by t%d\n", mutex_flag, thread_flag);
	//fprintf(stderr, "unlocked %p from %d\n", mutex_index[mutex_flag], thread_index[thread_flag]);

	//fprintf(stderr, "%d unlocks %p\n", (unsigned int)pthread_self(), mutex);
	
	//pthread_mutex_unlockp(&lock_mutex2);

	return temp;
}
