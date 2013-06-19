/** @file
	The input/output utilities used during execution:<ul>
	<li>Conversion of an integer into a string
	<li>Buffered read from a file descriptor
	<li>Write on a file descriptor, with possibility to 
	print all the results computed, or an integer value</ul>	
*/

#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include "io_utils.h"
#include "ipc_utils.h"

/// Constant buffer size
#define BUF_SIZE 512

static char read_char(int fd);

/**
	Converts an integer value into a string, which is stored
	into buffer.
	@param num The integer value to convert
	@param buffer The array where to save the converted number
	@param buf_len The maximum buffer length
	@return 0 in case of success, -1 if an error occurs
*/
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

/**
	Reads a line from the specified file descriptor and
	stores it in the passed array.
	@param fd The file descriptor
	@param dest The array where to save the line read
	@param max_length The maximum buffer length
	@return The line length in case of success, -1 if an error occurs
*/
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

/**
	Writes the results array on the specified output file.<br> 
	If the file does not exist, it's created.
	@param pathname The output file's path
	@param results The results array
	@param length The results array length
*/
void write_results(const char *const pathname, int *results, int length) {
	int fd, i;
	
	fd = open(pathname, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
	if(fd == -1) {
		write_to_fd(2, "Failed to open results file\n");
		kill_group(SIGTERM);
	}

	for(i = 0; i < length; ++i) {
		write_with_int(fd, "", results[i]);
	}
	
	if(close(fd) == -1) {
		write_to_fd(2, "Failed to close results file\n");
		kill_group(SIGTERM);
	}
}

/**
	Writes a string on the specified file descriptor, wrapping
	the write system call.
	@param fd The file descriptor
	@param s The string to write
*/
void write_to_fd(int fd, const char *const s) {
	if (s != NULL)
		if (write(fd, s, strlen(s) * sizeof(char)) == -1)
			write_to_fd(2, "Write failed\n"); 
}

/**
	Writes on the specified file descriptor a string followed 
	by an integer.
	@param fd The file descriptor
	@param s The string to write
	@param num The integer to write
*/
void write_with_int(int fd, const char *const s, int num) {
	char *message;
	char num_buffer[12];
	int message_len;
	if (itoa(num, num_buffer, 12) == -1)
		write_to_fd(2, "Failed to convert integer\n");
	message_len = strlen(s) + strlen(num_buffer) + 2;
	message = (char *) malloc(message_len * sizeof(char));
	if (message == NULL)
		write_to_fd(2, "Failed to allocate message string\n");
	strcpy(message, s);
	strcat(message, num_buffer);
	message[message_len - 2] = '\n';
	message[message_len - 1] = '\0';
	write_to_fd(fd, message);
	free(message);
}

/**
	Reads a character from the specified file descriptor, 
	wrapping the read system call. 
	The use of static variables allows to select the 
	correct character from the buffer.
	@param fd The file descriptor
	@return The char read
*/
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
