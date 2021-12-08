#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>

const unsigned long MAX_ARRAY_SIZE = 268435456; // 256 MB

void read_file(char *fname, long block_size) {
	struct timeval  tv1, tv2;
	gettimeofday(&tv1, NULL);
	// check file size in bytes
	struct stat st;
	stat(fname, &st);
	// read the file
	size_t bytesRead = 0;
    int fp = open (fname, O_RDONLY);
	double runtime;
	// if failed to read the data file
    if (fp == -1) {
        printf ("Input data file is invalid.\n");
        exit (10);
    }
    unsigned char *buffer = (unsigned char *) malloc(block_size * sizeof(char));
	int *myIntArray = (int *) malloc(MAX_ARRAY_SIZE * sizeof(int));
	if (myIntArray == NULL || buffer == NULL) {
        printf ("Unable to allocate memory.\n");
        exit (10);
	}
	unsigned long integters_stored = 0;
	int file_xor = 0;
	off_t end_position = lseek(fp, 0, SEEK_END);
	lseek (fp, 0, SEEK_SET);
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
	//printf("%ld\n", pos);
	close(fp);
	gettimeofday(&tv2, NULL);
	runtime = (double) (tv2.tv_usec - tv1.tv_usec) / 1000000 + (double) (tv2.tv_sec - tv1.tv_sec);
	printf("\nRead through-put with blocks-size %ld was: %f B/s (%f MiB/s)\n\n", 
						block_size, 
						st.st_size / runtime,
						st.st_size / (1024 * 1024 * runtime));
	printf("The XOR of all 4-byte integers in the file was %08x\n\n", file_xor);
}

int main(int argc, char* argv[]) {
   
	long blockSize = 0;
    char *filename = NULL;
	// process command line arguments
    // filename to read or write
	filename = argv[1];
	// size of each blocks
	blockSize = atol(argv[2]);
	// check if filename was provided
    if (!filename) {
        printf("Error! Please provide a file name.\n");
        return 2;
    }
	// check if block size was provided
    if (!blockSize) {
        printf("Error! Please provide block sizes to read.\n");
        return 3;
    }
	// do the requested operation
	read_file(filename, blockSize);
	// return successfully
	return 0;
}
