#define _GNU_SOURCE
#include <dlfcn.h>
#include <execinfo.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <string.h>

char * t[1024];
char * m[1024];
char thread[10][100] = {0};
char mutex[10][100] = {0};
int period[10] = {0};
char gate[1024][1024];
char ** lines = NULL;
char ** items = NULL;

int check_single(int i){
	int n = 1;

	while(n < i){	
		//if(t[0] != t[n]){
		if(strcmp(t[0],t[n]) != 0){
			return 0;
		}
		n++;
	}
	return 1;
}

int check_gate(int length, int count) {
	int x = 0; int i = 0; int j = 0;
	int flag = 0;
	int current_period = -1;
	char temp[10];

	for (i = 0; i < 10; i++) {
		if (strcmp(thread[i], t[0]) == 0) {
			current_period = period[i];
			strcpy(temp, mutex[i]);
			break;
		}
	}

	for (x = 1; x < length - 1; x++) {
		for (i = 0; i < 10; i++) {
			if (strcmp(thread[i], t[x]) == 0) {
				if (strcmp(temp, mutex[i]) != 0) {
					flag = -1;
					break;
				}
				if (current_period < period[i]) {
					flag = -1;
					break;
				}
			}
		}
	}

	if (flag == -1) return 0;

	return 1;
}

int check_segment(int length, int count) {
	int x = 0; int i = 0;
	int latest_period = 0;
	int orig_first_period = 0;

	for (i = 0; i < count; i++) {
			char * t_temp; char * m_temp;
			char gate_temp[1024];

			strcpy(gate_temp, gate[i]);

			t_temp = strtok(gate_temp, ",");
			m_temp = strtok(NULL, "\n");

		for (x = 1; x < length - 1; x++) {
			//printf("[%d] t: %s/%s, m: %s/%s\n", count, t_temp, t[x], m_temp, m[x]);
			
			if (strcmp(t_temp, t[x]) == 0 && strcmp(m_temp, m[x]) == 0) {
				//printf("[%d] t: %s/%s, m: %s/%s\n", count, t_temp, t[x], m_temp, m[x]);
				if (i + 1 > latest_period) latest_period = i + 1;
			}
		}
		//printf("m[length-1]: %s\n", m[length - 1]);
		if (strcmp(t_temp, t[0]) == 0 && strcmp(m_temp, m[length - 1]) == 0) {
			orig_first_period = i + 1;
		}
	}

	//printf("%d < %d\n", latest_period, orig_first_period);
	if (latest_period < orig_first_period) return 1;

	return 0;
}

int main(int argc, char **argv){

	char exec_file[50];
	char c;

	while ((c = getopt (argc, argv, "f:")) != -1) {
		switch (c){
			case 'f'://get execution file name
				strcpy(exec_file, strtok(argv[2], ":"));
				break;
		}
	}

	printf("%s\n", exec_file);

	int i = 0;
	FILE *fp = fopen("dmonitor.trace", "r");

	if(fp == NULL){
		printf("File open failed\n");
		exit(-1);
	}

	printf("> Start to predict DEADLOCK\n");

	lines = (char **)malloc(sizeof(char*));
	lines[0] = (char*)malloc(1024*sizeof(char));
	
	int count = 0;
	char buf[1024];
	while(fgets(buf, sizeof(buf),fp) != NULL){
		if (count == 0) {
			strcpy(lines[count], buf);
		} else {
			lines = (char **)realloc(lines, (count+1) * sizeof(*lines));
			lines[count] = (char *)malloc(1024*sizeof(char));
			strcpy(lines[count], buf);
		}
		count += 1;
		/*
		*/
	}

	int gate_count = 0;
	int a = 0;
	for (i = 0; i < count; i++) {
		int single_bit = 0; int gate_bit = 0; int segment_bit = 0;
		int j = 0;
		
		char *s1;
		char *s2;
		char *s3;

		s1 = strtok(lines[i], "[");
		s2 = strtok(NULL, "]");

		items = (char **)malloc(sizeof(char*));
		items[0] = (char*)malloc(64*sizeof(char));

		//printf("%s\n", s2);

		while((s3 = strtok(NULL, ">")) != NULL){
			if (j == 0) {
				strcpy(items[j], s3);
				strcpy(gate[gate_count], s3);
				gate_count++;
			}
			else {
				items = (char **)realloc(items, (j+1) * sizeof(*items));
				items[j] = (char *)malloc(64*sizeof(char));
				strcpy(items[j], s3);
			}
			j++;
		}

		int z = 0;
		int count_thread[10] = {0};
		for (z = 0; z < j; z++) {
			t[z] = strtok(items[z], ",");
			m[z] = strtok(NULL, "\n");
			int n = t[z][1] - 48;
			count_thread[n] = 1;
			if (z == 0) {
				if (a == 0) {
					strcpy(thread[a], t[z]);
					strcpy(mutex[a], m[z]);
					period[a] = i + 1;
					a += 1;
				} else {
					int b = 0;
					int limit = a;
					for (b = 0; b < limit; b++) {
						if (strcmp(thread[b], t[z]) == 0) break;
						if (b == a - 1) {
							strcpy(thread[a], t[z]);
							strcpy(mutex[a], m[z]);
							period[a] = i + 1;
							a += 1;
						}
					}
				}
			}
		}

		char command[50];

		if(j >= 2) {
			if (strcmp(t[0],t[j-1]) == 0) {
				single_bit = check_single(j);
				segment_bit = check_segment(j, i);
				if (j >= 3)
					gate_bit = check_gate(j, i);

				if (single_bit != 1 && segment_bit != 1 && gate_bit != 1) {
					printf("Potential Deadlock Found!\n");
					int cnt = 0;
					for (z = 0; z < 10; z++) {
						if (count_thread[z] == 1) cnt += 1;
					}
					printf("Number of Threads in Deadlock : %d Threads\n", cnt);
					printf("On the line of :\n");
					sprintf(command, "addr2line -e %s %s", exec_file, s2);
					system(command);
					printf("\n");
				}
			}
		}
		for (z = 0; z < j; z++) {
			char* currentCharPtr = items[z];
			free(currentCharPtr);
		}
		free(items);
	}
}
