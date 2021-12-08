#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>

const unsigned long MAX_ARRAY_SIZE = 268435456; // 256 MB

void write_file(char *fname, long block_size, long blocks) {
	// open urandom as read only file
    int fd1 = open("/dev/urandom", O_RDONLY);
    int fd2 = open(fname, O_CREAT | O_RDWR, S_IRWXU);
	// check for errors
    if (fd1 == -1 || fd2 == -1) {
        printf ("Error! Unable to open files.\n");
        exit (11);
	}
    char *buffer = malloc(block_size * sizeof(char));
	// read from /dev/urandom and write to required file until
	// all blocks are read and written.
    while (--blocks > 0)
    {
		read(fd1, buffer, block_size);
        write(fd2, buffer, block_size);
    }
    close(fd1);
    close(fd2);
}

int read_file(char *fname, long block_size) {
	// check file size in bytes
	struct stat st;
	stat(fname, &st);
	// read the file
	size_t bytesRead = 0;
    int fp = open (fname, O_RDONLY);
	unsigned char *buffer = (unsigned char *) malloc(block_size * sizeof(char));
	int *myIntArray = (int *) malloc(MAX_ARRAY_SIZE * sizeof(int));
	unsigned long integters_stored = 0;
	// if failed to read the data file
    if (fp == -1) {
        printf ("Input data file is invalid.\n");
        exit (10);
    }
    int file_xor = 0;
	while ((bytesRead = read(fp, buffer, block_size)) > 0)
	{
		for (int i = 0; i+3 < bytesRead; i += 4) {
			// XOR
			myIntArray[integters_stored%MAX_ARRAY_SIZE] = (buffer[i]<<24)^(buffer[i+1]<<16)^(buffer[i+2]<<8)^(buffer[i+3]);
			file_xor ^= *((unsigned int *)(buffer+i));//(buffer[i]<<24)^(buffer[i+1]<<16)^(buffer[i+2]<<8)^(buffer[i+3]);
			integters_stored++;
		}
	}
	close(fp);
	return file_xor;
}

int main(int argc, char* argv[]) {
    
	long blockSize = 0;
	size_t fileSize = 1024*1024*100;
    struct timeval  tv1, tv2;
	char *filename = "temp_file";
	double runtime;
	// process command line arguments
    // size of each blocks
	blockSize = atol(argv[1]);
	// check if block size was provided
    if (!blockSize) {
        printf("Error! Please provide block sizes to generate file size.\n");
        return 2;
    }
	// keep creating files until reasonable time is acheived
	int file_xor = 0;
	while (1) {
		// create a file with current size
		write_file(filename, blockSize, fileSize/blockSize);
		// read the file to see if the size is appropriate.
		gettimeofday(&tv1, NULL);
		file_xor = read_file(filename, blockSize);
		gettimeofday(&tv2, NULL);
		runtime = (double) (tv2.tv_usec - tv1.tv_usec) / 1000000 + (double) (tv2.tv_sec - tv1.tv_sec);
		if (runtime >= 5) break; // didn't put  && runtime <=15 because wanted to accomodate exceptional cases
		fileSize *= 2;
	}
	// clean up the temp file created
	if (remove(filename) != 0) {
        printf("Error! Failed to clean up.\n");
        return 3;
	}
	// print console message
	printf("\nSuccess !!!");
	printf("\nReasonable time of %f seconds achieved with \n\tfile size of %zu Bytes (%.2f GB)\n\n", //%zu specifier prints the length
				runtime, fileSize, fileSize / (1024.0 * 1024.0 * 1024.0));
	printf("\nThe XOR of all 4-byte integers in the file was %08x\n\n", file_xor);
	// return successfully
	return 0;
}

