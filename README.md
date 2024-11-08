# RUNNING BENCHMARKS WITH BACKGROUND memcpy()

This tool, runs a user provided benchmark, while having a number of threads performing memcpy in the background.

The size of the memcpy and the total memory per thread, the number of threads, the NUMA nodes of the benchmark and memcpy CPU and memory, as well as the time interval between each memcpy is provided using command line arguments.

## BUILDING

The NUMA library (libnuma) should be installed on the system.

In order to build the tool, you can use make. You can optionally set the CC and CFLAGS variables, in order to override the defaults.

## RUNNING

In order to run a benchmark with the tool, you have to run the following:

./noise [options] benchmark [benchmark arguments]

You can get details about the options by running:

./noise -h

For example, if the hpcg benchmark resides in the same directory as the noise tool, the command bellow:

./noise -s 20 -c 0 -m 0 -M 8 -T 11 -t 1000000 ./xhpcg

will use NUMA node 0 for the CPU of both the benchmark and the memcpy and the memory of the benchmark, while it will use node 8 for the memory of the memcpy.
It will spawn 11 threads that memcpy 1MB (1 << 20) and sleep for 1ms between each memcpy. Each threads allocates a total of 2MB.

## Example

There is a bash script (noise-stream.sh) in the example directory, that runs the stream benchmark with various configuration arguments.
In order to run it, we need to already have the noise tool and the stream benchmark compiled and invoke the script from the directory that has the noise and stream (stream_c.exe) binaries.
This can be done by running the build-stream.sh script, which compiles the noise and stream_c.exe binaries and copies them to the example directory.
We also provide the output of the script (noise-stream.log), when ran on a dual socket Intel(R) Xeon(R) CPU Max 9468 computer.
