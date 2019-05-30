#!/bin/bash
for i in {1..1000}
do
	LD_PRELOAD=./ddetector.so ./dinning_deadlock 2>> error.txt
	echo ""
	echo "" >> error.txt
done
