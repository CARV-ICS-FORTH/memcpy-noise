CFLAGS ?= -O3 -flto -march=native -mcpu=native -mtune=native

noise: noise.c
	$(CC) $(CFLAGS) -o noise noise.c -lnuma -lpthread

clean:
	rm -f noise
