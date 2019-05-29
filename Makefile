all:
	gcc -pthread -o abba abba.c
	gcc -pthread -o extraabba extraabba.c
	gcc -pthread -o dinning_deadlock dinning_deadlock.c
	gcc -pthread -o dinning dinning.c
	gcc -pthread -shared -fPIC -o ddetector.so ddetector.c -ldl
