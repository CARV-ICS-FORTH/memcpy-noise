#!/bin/bash

export OMP_NUM_THREADS=6

#In all cases of this example  we use 6 openMP threads for stream and numa node 0 for cpu for both the benchmark and the noise
#We allocate 1024 (1 << 10) chunks of 1MB (1 << 20) per noise thread

#First we have some configurations with both memory allocations happening at node 0 (DDR on Intel Xeon MAX)

#-T 0 means that we have no noise threads. the output would be similar to numactl -N 0 -m 0 ./stream_c.exe
echo "./noise -T 0 -s 20 -n 10 -c 0 -m 0 -t 0 ./stream_c.exe"
./noise -T 0 -s 20 -n 10 -c 0 -m 0 -t 0 ./stream_c.exe
echo

#Below we have 6 noise threads with no interval between each memcpy
echo "./noise -T 6 -s 20 -n 10 -c 0 -m 0 -t 0 ./stream_c.exe"
./noise -T 6 -s 20 -n 10 -c 0 -m 0 -t 0 ./stream_c.exe
echo

#Below the interval between each memcpy is 1 us
echo "./noise -T 6 -s 20 -n 10 -c 0 -m 0 -t 1000 ./stream_c.exe"
./noise -T 6 -s 20 -n 10 -c 0 -m 0 -t 1000 ./stream_c.exe
echo

#Below the interval between each memcpy is 1 ms
echo "./noise -T 6 -s 20 -n 10 -c 0 -m 0 -t 1000000 ./stream_c.exe"
./noise -T 6 -s 20 -n 10 -c 0 -m 0 -t 1000000 ./stream_c.exe
echo

#Below we have 6 noise threads with no interval between each memcpy. However the noise memory is on node 8 (HBM on Intel Xeon MAX) and the banchmark on node 0
echo "./noise -T 6 -s 20 -n 10 -c 0 -m 0 -M 8 -t 0 ./stream_c.exe"
./noise -T 6 -s 20 -n 10 -c 0 -m 0 -M 8 -t 0 ./stream_c.exe
echo

#The configurations bellow use memory node 8 (HBM on Intel Xeon MAX) for both the benchmark and the noise

#-T 0 means that we have no noise threads. the output would be similar to numactl -N 0 -m 8 ./stream_c.exe
echo "./noise -T 0 -s 20 -n 10 -c 0 -m 8 -t 0 ./stream_c.exe"
./noise -T 0 -s 20 -n 10 -c 0 -m 8 -t 0 ./stream_c.exe
echo

#Below we have 6 noise threads with no interval between each memcpy
echo "./noise -T 6 -s 20 -n 10 -c 0 -m 8 -t 0 ./stream_c.exe"
./noise -T 6 -s 20 -n 10 -c 0 -m 8 -t 0 ./stream_c.exe
echo
