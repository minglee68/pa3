all:
	gcc -pthread -o abba abba.c
	gcc -pthread -o gatelock gatelock.c
	gcc -pthread -o segmentlock segmentlock.c
	gcc -pthread -o dinning_deadlock dinning_deadlock.c
	gcc -pthread -o dinning dinning.c
	gcc -pthread -o gabba -g abba.c
	gcc -pthread -o ggatelock -g gatelock.c
	gcc -pthread -o gsegmentlock -g segmentlock.c
	gcc -pthread -o gdinning_deadlock -g dinning_deadlock.c
	gcc -pthread -o gdinning -g dinning.c
	gcc -pthread -shared -fPIC -o ddetector.so ddetector.c -ldl
	gcc -pthread -shared -fPIC -o dmonitor.so -g dmonitor.c -ldl
