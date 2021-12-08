#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>

const unsigned long MAX_ARRAY_SIZE = 268435456; // 256 MB

void write_file(char *fname, long block_size, long blocks) {
	struct timeval  tv1, tv2;
	gettimeofday(&tv1, NULL);
	
	// open urandom as read only file
    int fd1 = open("/dev/urandom", O_RDONLY);
    int fd2 = open(fname, O_CREAT | O_RDWR, S_IRWXU);
	double runtime;
	long local_blocks = blocks;
	
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
	gettimeofday(&tv2, NULL);
	runtime = (double) (tv2.tv_usec - tv1.tv_usec) / 1000000 + (double) (tv2.tv_sec - tv1.tv_sec);
	double total_size = block_size*local_blocks;
	
	printf("\nFile Created:");
	printf("\n\tFile name   : %s", fname);
	printf("\n\t# of blocks : %ld", local_blocks);
	printf("\n\tBlocks size : %ld Bytes", block_size);
	printf("\n\tTotal size  : %.0f B (%.2f MB)", total_size, total_size/1048576); //1048576 is # of bytes in a Megabyte
	printf("\n\tWrite time  : %.2f seconds", runtime);
	printf("\n\tWrite speed : %.2f B/s (%.2f MiB/s)\n\n", total_size/runtime, total_size/(runtime*1048576));
}

void read_file(char *fname, long block_size) {
	struct timeval  tv1, tv2;
	gettimeofday(&tv1, NULL);
	// check file size in bytes
	struct stat st;
	stat(fname, &st);
	// read the file
	size_t bytesRead = 0;
    int fp = open (fname, O_RDONLY);
	unsigned char *buffer = (unsigned char *) malloc(block_size * sizeof(char));
	int *myIntArray = (int *) malloc(MAX_ARRAY_SIZE * sizeof(int));
	if (myIntArray == NULL || buffer == NULL) {
        printf ("Unable to allocate memory.\n");
        exit (10);
	}
	unsigned long integters_stored = 0;
	double runtime;
	// if failed to read the data file
    if (fp == -1 ) {
        printf ("Input data file is invalid.\n");
        exit (10);
    }
    int file_xor = 0;
    	//long int pos = 0;
    	
	while ((bytesRead = read(fp, buffer, block_size)) > 0)
	{
		for (int i = 0; i+3 < bytesRead; i += 4) {
			// XOR
			myIntArray[integters_stored%MAX_ARRAY_SIZE] = (buffer[i]<<24)^(buffer[i+1]<<16)^(buffer[i+2]<<8)^(buffer[i+3]);
			
			file_xor ^= *((unsigned int *)(buffer+i));//(buffer[i]<<24)^(buffer[i+1]<<16)^(buffer[i+2]<<8)^(buffer[i+3]);
			
			integters_stored++;
		}
		//pos += bytesRead;
		//printf("%ld,%ld\n", pos, bytesRead);
	}
	close(fp);
	gettimeofday(&tv2, NULL);
	runtime = (double) (tv2.tv_usec - tv1.tv_usec) / 1000000 + (double) (tv2.tv_sec - tv1.tv_sec);
	printf("\nRead through-put with blocks-size %ld was: %.2f B/s (%.2f MiB/s)\n\n", 
						block_size, 
						st.st_size / runtime,
						st.st_size / (1024 * 1024 * runtime));
	printf("The XOR of all 4-byte integers in the file was %08x\n\n", file_xor);
}

int main(int argc, char* argv[]) {
    
    long blockCount = 0;
	long blockSize = 0;
	char option = '\0';
    char *filename = NULL;
	// get command line arguments expected format
	// ./code [filename] [-r|-w] <block_size> <block_count>
	// filename to read or write
	filename = argv[1];
	// read / write choice flag
	if (strcmp(argv[2], "-r") == 0) {
		option = 'r';
	} else if (strcmp(argv[2], "-w") == 0) {
		option = 'w';
	} else {
		printf("Error! Please provide either -w or -r flag.\n");
		return 1;
	}
	// size of each blocks
	blockSize = atol(argv[3]);
	// # of blocks to process
	blockCount = atol(argv[4]);
	// check if filename was provided
    if (!filename) {
        printf("Error! Please provide a file name.\n");
        return 2;
    }
	// check if write size was provided
    if (option == 'w' && (!blockCount || !blockSize)) {
        printf("Error! Please provide block count and block size for write operation.\n");
        return 3;
    }
	// check if block size was provided
    if (option == 'r' && !blockSize) {
        printf("Error! Please provide block sizes for read operation.\n");
        return 4;
    }
	// do the requested operation
	if (option == 'w') {
		write_file(filename, blockSize, blockCount);
	} else if (option == 'r') {
		read_file(filename, blockSize);
	}
	// return successfully
	return 0;
}
