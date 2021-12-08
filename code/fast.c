#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <inttypes.h>

const int MAX_THREADS = 8; 	// maximum threads to run
unsigned long MAX_ARRAY_SIZE = 0; 		
unsigned int file_xor = 0;
// create structure to pass data to worker_threads
typedef struct {
	size_t bytesRead;
	unsigned char *buffer;
	unsigned int part_xor;
} args_worker_threads;

void *worker_threads(void *args) {
	args_worker_threads *args_r = args;
	size_t bytesRead = args_r->bytesRead;
	unsigned char *buffer = args_r->buffer;
	int *myIntArray = (int *) malloc((bytesRead/4) * sizeof(int));
	int integters_stored = 0;
	unsigned int part_xor = 0;
	// create a thread to this work
	for (int i = 0; i+3 < bytesRead; i += 4) {
		// XOR
		myIntArray[integters_stored%MAX_ARRAY_SIZE] = (buffer[i])^(buffer[i+1])^(buffer[i+2])^(buffer[i+3]);
		part_xor ^= *((unsigned int *)(buffer+i));//(buffer[i]<<24)^(buffer[i+1]<<16)^(buffer[i+2]<<8)^(buffer[i+3]);
		integters_stored++;
		
	}
	
	//args_r->part_xor = part_xor;
	//printf("%d,%u\n",integters_stored, part_xor);
	free(myIntArray);
	
	return (void*)((uintptr_t)part_xor);
}

void read_file(char *fname) {
	struct timeval  tv1, tv2;
	gettimeofday(&tv1, NULL);
	// check file size in bytes
	struct stat st;
	stat(fname, &st);
	
	long int block_size = (int)(st.st_size / MAX_THREADS);
	
	while(block_size && block_size % 4) {
		block_size++;
	}
	
	//printf("%ld,%ld\n", st.st_size,block_size);
	//return;
	//printf("%ld",block_size);
	// read the file
	size_t bytesRead = 0;
    int fp = open (fname, O_RDONLY);
	unsigned char *buffer[MAX_THREADS];
	int th_count = 0;
	pthread_t tid[MAX_THREADS];
	unsigned long integters_stored = 0;
	double runtime;
	// if failed to read the data file
    if (fp == -1) {
        printf ("Input data file is invalid.\n");
        exit (10);
    }
	// create new buffer so as not to overwrite the one being 
	// read by worker thread
	buffer[th_count%MAX_THREADS] = (unsigned char *) malloc(block_size * sizeof(char));
	
	while ((bytesRead = read(fp, buffer[th_count%MAX_THREADS], block_size)) > 0)
	{
		// setup send parameters
		args_worker_threads *pass_arg = malloc(sizeof *pass_arg);
		pass_arg->buffer = buffer[th_count%MAX_THREADS];
		pass_arg->bytesRead = bytesRead;
		// create an instance of worker thread.
		pthread_create(&tid[th_count%MAX_THREADS], NULL, worker_threads, pass_arg);
		th_count++;
		// wait for one thread to terminate
		if (th_count >= MAX_THREADS) {
			unsigned int returnValue;
			pthread_join(tid[th_count%MAX_THREADS], (void*)&returnValue);
			//printf("closed %d thread: %u\n", th_count%MAX_THREADS, returnValue);
			file_xor ^= returnValue;
		} else {
			// create new buffer so as not to overwrite the one being 
			// read by worker thread
			buffer[th_count%MAX_THREADS] = (unsigned char *) malloc(block_size * sizeof(char));
		}
	}
	for (int j = 0; j < MAX_THREADS - 1; j++) {
		th_count++;
		unsigned int returnValue;
		pthread_join(tid[th_count%MAX_THREADS], (void*)&returnValue);		
		// printf("closed %d thread: %u\n", th_count%MAX_THREADS, returnValue);
		file_xor ^= returnValue;
	}
	gettimeofday(&tv2, NULL);
	runtime = (double) (tv2.tv_usec - tv1.tv_usec) / 1000000 + (double) (tv2.tv_sec - tv1.tv_sec);
	printf("\nRead through-put with blocks-size %ld was: %f B/s (%f MiB/s)\n\n", 
						block_size, 
						st.st_size / runtime,
						st.st_size / (1024 * 1024 * runtime));
	printf("The XOR of all 4-byte integers in the file was %08x\n\n", file_xor);
	close(fp);
}

int main(int argc, char* argv[]) {

    char *filename = NULL;
	MAX_ARRAY_SIZE = 268435456 / MAX_THREADS;
	// process command line arguments
    // filename to read or write
	filename = argv[1];

	// check if filename was provided
    if (!filename) {
        printf("Error! Please provide a file name.\n");
        return 2;
    }

	// perform the read operation in parallel.
	read_file(filename);
	// return successfully
	return 0;
}
