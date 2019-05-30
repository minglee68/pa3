all:
	gcc -pthread -o abba abba.c
	gcc -pthread -o gateabba gateabba.c
	gcc -pthread -o segmentabba segmentabba.c
	gcc -pthread -o dinning_deadlock dinning_deadlock.c
	gcc -pthread -o dinning dinning.c
	gcc -pthread -o gabba -g abba.c
	gcc -pthread -o ggateabba -g gateabba.c
	gcc -pthread -o gsegmentabba -g segmentabba.c
	gcc -pthread -o gdinning_deadlock -g dinning_deadlock.c
	gcc -pthread -o gdinning -g dinning.c
	gcc -pthread -shared -fPIC -o ddetector.so ddetector.c -ldl
	gcc -pthread -shared -fPIC -o dmonitor.so -g dmonitor.c -ldl
