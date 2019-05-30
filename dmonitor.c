#define _GNU_SOURCE
#include <dlfcn.h>
#include <execinfo.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <string.h>

static pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;

static int lock[10][100];
static int try[100][10];

static pthread_mutex_t *mutex_index[100] = {NULL};
static unsigned int thread_index[10] = {0};

void check_lock(int orig_thread, int mutex_flag, char * output, int thread_flag, FILE * fp);
void check_try(int orig_thread, int thread_flag, char * output, int mutex_flag, FILE * fp);

void check_lock(int orig_thread, int mutex_flag, char * output, int thread_flag, FILE * fp) {
	int i = 0;
	int result = 0;
	for(i = 0; i < 10; i++) {
		char temp[10000] = "";
		if (lock[i][mutex_flag] == 1) {
			if (i == thread_flag) continue;
			strcpy(temp, output);
			//fprintf(stderr, "->lock on m%d by t%d(%d)", mutex_flag, i, orig_thread);
			sprintf(temp, "%s>t%d,m%d", output, i, mutex_flag);
			if (i == orig_thread) {
				fprintf(fp, "%s\n", temp);
				return ;
			}
			//	return -1;
			//return check_try(orig_thread, i, output, mutex_flag);
			check_try(orig_thread, i, temp, mutex_flag, fp);
			//if (result == -1) return -1;
		}
	}

	fprintf(fp, "%s\n", output);
}

void check_try(int orig_thread, int thread_flag, char * output, int mutex_flag, FILE * fp) {
	int i = 0;
	int result = 0;
	char temp[10000] = "";
	for (i = 0; i < 100; i++) {
		if (try[i][thread_flag] == 1) {
			if (i == mutex_flag) continue;
			strcpy(temp, output);
			sprintf(temp, "%s>t%d,m%d", output, thread_flag, i);
			//fprintf(stderr, "->try by t%d on m%d(%d)", thread_flag, i, orig_thread);
			//return check_lock(orig_thread, i, output, thread_flag);
			check_lock(orig_thread, i, temp, thread_flag, fp);
			//if (result == -1) return -1;
		}
	}
}

int pthread_mutex_lock(pthread_mutex_t *mutex) {

	static __thread char addr[100] = "";
	static __thread int n_malloc = 0;
	n_malloc += 1;
	static __thread int backtrace_index = -1;

	int i = 0; int j = 0;

	int (*pthread_mutex_lockp)(pthread_mutex_t *mutex);
	int (*pthread_mutex_unlockp)(pthread_mutex_t *mutex);
	char * error;
	
	FILE* fp;
	fp = fopen("dmonitor.trace", "a+");

	char output[10000];

	pthread_mutex_lockp = dlsym(RTLD_NEXT, "pthread_mutex_lock");
	if ((error = dlerror()) != 0x0)
		exit(1);

	pthread_mutex_unlockp = dlsym(RTLD_NEXT, "pthread_mutex_unlock");
	if ((error = dlerror()) != 0x0)
		exit(1);

	if (n_malloc == 1) {
		void * arr[10] ;
		char ** stack ;

		fprintf(stderr, "pthread_mutex_lock(%p):%d\n", mutex, (unsigned int) pthread_self()) ;

		size_t sz = backtrace(arr, 10) ;
		stack = backtrace_symbols(arr, sz) ;

		fprintf(stderr, "Stack trace\n") ;
		fprintf(stderr, "============\n") ;
		for (i = 0 ; i < sz ; i++)
			if (i == 1) {
				sprintf(addr, "%s", stack[i]) ;
			}
		fprintf(stderr, "============\n\n") ;
	}

	int thread_flag = -1;
	int mutex_flag = -1;
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

	if (strlen(addr) < 10) {
		backtrace_index = mutex_flag;
	}

	sprintf(output, "%s", addr);

	pthread_mutex_lockp(&mutex1);

	if (strlen(addr) > 10 && mutex_flag != backtrace_index) {

		if (try[mutex_flag][thread_flag] != 1) {
			try[mutex_flag][thread_flag] = 1;
		}

		//fprintf(stderr, "(start %d)try by t%d on m%d", thread_flag, thread_flag, mutex_flag);
		sprintf(output, "%s>t%d,m%d", output, thread_flag, mutex_flag);
		check_lock(thread_flag, mutex_flag, output, -1, fp);
			//fprintf(stderr, "\nDeadlock Detected!(%d)", thread_flag);
		//fprintf(stderr, "(end %d)\n", thread_flag);
/*
		fprintf(stderr, "%s\n", output);

		fprintf(fp, "%s\n", output);
*/
	}

	fclose(fp);

	pthread_mutex_unlockp(&mutex1);

	int temp = pthread_mutex_lockp(mutex);

	pthread_mutex_lockp(&mutex1);

	if (strlen(addr) > 10 && mutex_flag != backtrace_index) {
		lock[thread_flag][mutex_flag] = 1;
		//try[mutex_flag][thread_flag] = 0;
	}

	//fprintf(stderr, "lock on m%d by t%d\n", mutex_flag, thread_flag);

	pthread_mutex_unlockp(&mutex1);

	n_malloc -= 1;

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

	pthread_mutex_lockp(&mutex1);

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

	//lock[thread_flag][mutex_flag] = 0;

	//fprintf(stderr, "unlock on m%d by t%d\n", mutex_flag, thread_flag);

	pthread_mutex_unlockp(&mutex1);

	return temp;
}
