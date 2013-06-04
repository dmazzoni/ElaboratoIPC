#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "io_utils.h"

#define MAX_LENGTH 50
#define BUF_SIZE 512

static int read_line(int fd, char *const dest);
static char read_char(int fd);

list* parse_file(const char *const pathname) {
	list *result = list_construct();
	char line[MAX_LENGTH];
	int len, fd;
	
	if(!result) 
		exit(1);

	fd = open(pathname, O_RDONLY);
	if(fd == -1) {
		perror("Failed to open setup file");
		exit(1);
	}
		
	do {
		len = read_line(fd, line);
		if (len > 0)
			list_append(result, line);
	} while(len >= 0);
	
	if (close(fd) == -1) {
		perror("Failed to close setup file");
		exit(1);
	}

	return result;
}

void write_err(const char *const message) {
	if (write(2, message, strlen(message) * sizeof(char)) < 0)
		perror("Write to stderr failed"); 
}

static int read_line(int fd, char *const dest) {
	int i = 0;
	char c;
	
	while (i < MAX_LENGTH) {
		c = read_char(fd);
		if (c == EOF) {
			dest[i] = '\0';
			return -1;
		} else if (c == '\n') {
			dest[i] = '\0';
			return i;	
		} else
			dest[i++] = c;
	}
	
	write_err("Buffer overflow\n");
	exit(1);
}

static char read_char(int fd) {
	static char buffer[BUF_SIZE];
	static int chars_left = 0;
	static int i = 0;
	
	if (chars_left == 0) {
		chars_left = read(fd, &buffer, BUF_SIZE * sizeof(char));
		i = 0;
		if (chars_left == 0)
			return EOF;
		if (chars_left == -1) {
			write_err("Failed to read from file\n");
			exit(1);
		}
	}
	--chars_left;
	return buffer[i++];
}
