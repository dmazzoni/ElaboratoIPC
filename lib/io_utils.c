#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include "io_utils.h"

#define MAX_LENGTH 50
#define BUF_SIZE 512

static int read_line(int fd, char *const dest);
static char read_char(int fd);

int itoa(int num, char *const buffer, int buf_len) {
	int i = 0, j = 0;
	char temp;
	
	if (num == 0)
		buffer[i++] = '0';
	if (num < 0) {
		buffer[i++] = '-';
		num *= -1;
	}
	while ((i < buf_len - 1) && (num > 0)) {
		buffer[i++] = num % 10 + '0';
		num /= 10;
	}
	if (num > 0)
		return -1;
	buffer[i--] = '\0';
	while (j < i) {
		temp = buffer[i];
		buffer[i--] = buffer[j];
		buffer[j++] = temp;
	}
	return 0;	
}

list* parse_file(const char *const pathname) {
	list *result = list_construct();
	char line[MAX_LENGTH];
	int len, fd;
	
	if(result == NULL) 
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

void write_results(const char *const pathname, int *results, int length) {
	int fd, i;
	char buffer[12];
	
	fd = open(pathname, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
	if(fd == -1) {
		perror("Failed to open results file");
		kill(0, SIGTERM);
	}

	for(i = 0; i < length; ++i) {
		if(itoa(results[i], buffer, 12) == -1)
			write_to_fd(2, "Failed to convert result\n");
		else {
			write_to_fd(fd, buffer);
			write_to_fd(fd, "\n");
		}
	}
	
	if(close(fd) == -1) {
		perror("Failed to close results file");
		kill(0, SIGTERM);
	}
}

void write_to_fd(int fd, const char *const s) {
	if (write(fd, s, strlen(s) * sizeof(char)) == -1)
		perror("Write failed"); 
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
	
	write_to_fd(2, "Buffer overflow\n");
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
			write_to_fd(2, "Failed to read from file\n");
			exit(1);
		}
	}
	--chars_left;
	return buffer[i++];
}
