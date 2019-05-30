#!/bin/bash

rm dmonitor.trace
LD_PRELOAD=./dmonitor.so ./ggateabba
