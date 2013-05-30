#include <fcntl.h> 
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "lib/list.h"
#include "lib/operation.h"



int main(int argc, char *argv[]) {
	int fd;
	
	if(argc != 2)
		if(write(stderr, "Usage: main.x <pathname>\n", 25 * sizeof(char)) < 0)
			perror("Write"); 
		
	fd = open(argv[1], O_RDONLY);
	if(fd == -1) {
		perror("Failed to open setup file");
		exit(1);
	}
	
}
