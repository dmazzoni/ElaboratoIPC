#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include "io_utils.h"

#define BUF_SIZE 512

static char read_char(int fd);

int itoa(int num, char *const buffer, int buf_len) {
	int i = 0, j = 0;
	char temp;
	
	if (num == 0)
		buffer[i++] = '0';
	if (num < 0) {
		buffer[i++] = '-';
		num *= -1;
		j++;
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

int read_line(int fd, char *const dest, const int max_length) {
	int i = 0;
	char c;
	
	while (i < max_length) {
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

void write_results(const char *const pathname, int *results, int length) {
	int fd, i;
	
	fd = open(pathname, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
	if(fd == -1) {
		perror("Failed to open results file");
		kill(0, SIGTERM);
	}

	for(i = 0; i < length; ++i) {
		write_with_int(fd, NULL, results[i]);
	}
	
	if(close(fd) == -1) {
		perror("Failed to close results file");
		kill(0, SIGTERM);
	}
}

void write_to_fd(int fd, const char *const s) {
	if (s != NULL)
		if (write(fd, s, strlen(s) * sizeof(char)) == -1)
			perror("Write failed"); 
}

void write_with_int(int fd, const char *const s, int num) {
	char buffer[12];
	write_to_fd(fd, s);
	if (itoa(num, buffer, 12) == -1)
		write_to_fd(2, "Failed to convert integer\n");
	else {
		write_to_fd(fd, buffer);
		write_to_fd(fd, "\n");
	}
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
