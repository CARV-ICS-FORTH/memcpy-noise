#RUNNING BENCHMARKS WITH BACKGROUND memcpy()

This tool, runs a user provided benchmark, while having a number of threads performing memcpy in the background.

The size of the memcpy and the total memory per thread, the number of threads, the numa nodes of the benchbmark and memcpy cpu and memory, as well as the time interval between each memcpy is provided using command line arguments.

## BUILDING

The numa library (libnuma) should be installed on the system.

In order to build the tool, you can use make. You can optionally set the CC and CFLAGS variables, in order to override the defaults.

## RUNNING

In order to run a benchmark with the tool, you have to run the following:

./noise [options] benchmark [benchmark arguments]

options:
-c <nodes> : the cpu numa nodes where the benchmark will be run, siminlar to numactl -c (defult current behaviour)
-C <nodes> : the cpu numa nodes where the noise will be run (defult same as -c)
-m <nodes> : the memory numa nodes where the benchmark will be run (defult current behaviour)
-M <nodes> : the memory numa nodes where the noise will be run (defult same as -m)
-t <nsecs> : the interval between each noise memcpy (default 0)
-T <threads> : the number of noise threads (default 0)
-s <shift> : noise chunk size will be 1 << shift (default 12 => 4KB)
-n <shift> : noise chunks number will be 1 << shift (default 1 => 2 chunks)

For example, if the hpcg benchmark resides in the same directory as the nois tool, the command bellow:

./noise -s 20 -c 0 -m 0 -M 8 -T 11 -t 1000000 ./xhpcg

will use numa node 0 for the cpu of both the benchmark and the memcpy and the memory of the banchmark, while it will use node 8 for the memory of the memcpy.
It will spawn 11 threads that memcpy 1MB (1 << 20) and sleep for 1ms between each memcpy. Each threads allocates a total of 2MB.
