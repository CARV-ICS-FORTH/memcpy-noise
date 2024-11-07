#include <string.h>
#include <numaif.h>
#include <numa.h>
#include <errno.h>
#include <time.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <stdio.h>
#include <unistd.h>

struct bitmask *noiseCpu = NULL, *noiseMem = NULL, *benchCpu = NULL, *benchMem = NULL;
long noiseChunks = -1, noiseChunkOrder = -1;
struct timespec noiseInterval = {-1, -1};
long threads = -1;
char **benchBin;
size_t chunkSize, totalSize, chunkMask;
sem_t barrier;

void printUsage(char *name){
	fprintf(stderr, "Usage: %s [options] benchmark [benchmark arguments]\n\noptions:\n", name);
	fprintf(stderr, "-c <nodes> : the cpu numa nodes where the benchmark will be run, similar to numactl -N (defult current behaviour)\n");
	fprintf(stderr, "-C <nodes> : the cpu numa nodes where the noise will be run (defult same as -c)\n");
	fprintf(stderr, "-m <nodes> : the memory numa nodes where the benchmark will be run, similar to numactl -m (defult current behaviour)\n");
	fprintf(stderr, "-M <nodes> : the memory numa nodes where the noise will be run (defult same as -m)\n");
	fprintf(stderr, "-t <nsecs> : the interval between each noise memcpy (default 0)\n");
	fprintf(stderr, "-T <threads> : the number of noise threads (default 0)\n");
	fprintf(stderr, "-s <shift> : noise chunk size will be 1 << shift (default 12 => 4KB)\n");
	fprintf(stderr, "-n <shift> : noise chunks number will be 1 << shift (default 1 => 2 chunks)\n");
}

int parseOpts(int argc, char **argv){
	int i;
	for(i = 1; (i < argc - 1) && (argv[i][0] == '-'); i += 2){
		if(!strcmp(argv[i], "-c")){
			if(benchCpu == NULL){
				benchCpu = numa_parse_nodestring(argv[i+1]);
				if(benchCpu == NULL || benchCpu == numa_no_nodes_ptr){
					goto inval;
				}
			}else{
				goto redef;
			}
		}else if(!strcmp(argv[i], "-m")){
			if(benchMem == NULL){
				benchMem = numa_parse_nodestring(argv[i+1]);
				if(benchMem == NULL || benchMem == numa_no_nodes_ptr){
					goto inval;
				}
			}else{
				goto redef;
			}
		}else if(!strcmp(argv[i], "-C")){
			if(noiseCpu == NULL){
				noiseCpu = numa_parse_nodestring(argv[i+1]);
				if(noiseCpu == NULL || noiseCpu == numa_no_nodes_ptr){
					goto inval;
				}
			}else{
				goto redef;
			}
		}
		else if(!strcmp(argv[i], "-M")){
			if(noiseMem == NULL){
				noiseMem = numa_parse_nodestring(argv[i+1]);
				if(noiseMem == NULL || noiseMem == numa_no_nodes_ptr){
					goto inval;
				}
			}else{
				goto redef;
			}
		}else if(!strcmp(argv[i], "-t")){
			if(noiseInterval.tv_sec == -1){
				noiseInterval.tv_nsec = strtol(argv[i+1], NULL, 0);
				if(noiseInterval.tv_nsec < 0){
					goto inval;
				}
				/* WONTFIX support interval >= (1 << 64) nsec*/
				noiseInterval.tv_sec = noiseInterval.tv_nsec/1000000000;
				noiseInterval.tv_nsec %= 1000000000;
			}else{
				goto redef;
			}
		}else if(!strcmp(argv[i], "-T")){
			if(threads == -1){
				threads = strtol(argv[i+1], NULL, 0);
				if(threads < 0){
					goto inval;
				}
			}else{
				goto redef;
			}
		}else if(!strcmp(argv[i], "-s")){
			if(noiseChunkOrder == -1){
				noiseChunkOrder = strtol(argv[i+1], NULL, 0);
				if(noiseChunkOrder < 0){
					goto inval;
				}
			}else{
				goto redef;
			}
		}else if(!strcmp(argv[i], "-n")){
			if(noiseChunks == -1){
				noiseChunks = strtol(argv[i+1], NULL, 0);
				if(noiseChunks < 1){
					goto inval;
				}
			}else{
				goto redef;
			}
		}else{
			fprintf(stderr, "Invalid option: %s\n", argv[i]);
			printUsage(argv[0]);
			return -EINVAL;
		}
	}

	if(argv[i][0] == '-'){
		fprintf(stderr, "%s option has no argument\n\n", argv[i]);
		printUsage(argv[0]);
		return -EINVAL;
	}

	if(noiseCpu == NULL){
		noiseCpu = benchCpu;
	}

	if(noiseMem == NULL){
		benchMem = benchMem;
	}

	if(noiseInterval.tv_sec == -1){
		noiseInterval.tv_sec = 0;
		noiseInterval.tv_nsec = 0;
	}

	if(threads == -1){
		threads = 0;
	}

	if(noiseChunkOrder == -1){
		noiseChunkOrder = 12;
	}

	if(noiseChunks == -1){
		noiseChunks = 1;
	}

	chunkSize = 1 << noiseChunkOrder;
	totalSize = chunkSize << noiseChunks;
	chunkMask = totalSize - 1;
	benchBin = argv + i;
	return 0;


	redef:
		fprintf(stderr, "%s option defined mere than once\n\n", argv[i]);
		printUsage(argv[0]);
		return -EINVAL;
	inval:
		fprintf(stderr, "%s option has an invalid argument\n\n", argv[i]);
		printUsage(argv[0]);
		return -EINVAL;
}

void *noise(void * unused){
	void *base;
	size_t fromOfft = 0, toOfft;
	base = aligned_alloc((chunkSize>getpagesize())?chunkSize:getpagesize(), (totalSize>getpagesize())?totalSize:getpagesize());
	if(base == NULL){
		fprintf(stderr, "noise memory allocation failed\n");
		return NULL;
	}
	if(noiseMem != NULL && mbind(base, (totalSize>getpagesize())?totalSize:getpagesize(), MPOL_BIND, noiseMem->maskp, noiseMem->size, MPOL_MF_MOVE)){
		perror("mbind()");
		return NULL;
	}
	if(noiseCpu != NULL && numa_run_on_node_mask_all(noiseCpu)){
		perror("numa_run_on_node_mask_all()");
		return NULL;
	}
	toOfft = totalSize >> 1;
	memset(base, 0xff, totalSize);
	sem_post(&barrier);
	while(1){
		memcpy(base + toOfft, base + fromOfft, chunkSize);
		toOfft = ((toOfft + chunkSize) & chunkMask);
		fromOfft = ((fromOfft + chunkSize) & chunkMask);
		if(noiseInterval.tv_sec || noiseInterval.tv_nsec){
			nanosleep(&noiseInterval, NULL);
		}
	}
}

int runBench(char **argv){
	pid_t pid;
	pid = fork();
	if(pid < 0){
		perror("fork()");
		return -errno;
	}else if(pid == 0){
		if(benchCpu != NULL && numa_run_on_node_mask_all(benchCpu)){
			perror("numa_run_on_node_mask_all()");
			return -errno;
		}
		if(benchMem != NULL){
			numa_set_membind(benchMem);
		}
		execv(argv[0], argv);
		perror("execv()");
		return -errno;
	}else{
		/*parent*/
		int status;

		if(waitpid(pid, &status, 0) < 0){
			perror("waitpid()");
			return -errno;
		}
		if(WIFEXITED(status)){
			return -WEXITSTATUS(status);
		}else{
			return -EINTR;
		}
	}
}

int main(int argc, char **argv){
	int err, i;
	pthread_t unused;
	if(argc == 2 && !strcmp(argv[1], "-h")){
		printUsage(argv[0]);
		return 0;
	}
	if((err = parseOpts(argc, argv))){
		return -err;
	}
	sem_init(&barrier, 0, 0);
	for(i = 0; i < threads; i++){
		if(pthread_create(&unused, NULL, noise, NULL)){
			perror("pthread_create()");
			return errno;
		}
	}
	for(i = 0; i < threads; i++){
		while(sem_wait(&barrier));
	}
	sem_destroy(&barrier);
	return -runBench(benchBin);
}
