#include <string.h>
#include "io_utils.h"

#define MAX_LENGTH 50
#define BUF_SIZE 512

static char read_line(int fd, char *buffer);

list* parse_file(int fd) {
	list *result = list_construct();
	char line[MAX_LENGTH], c;
	
	if(!result)
		return NULL;
		
	do {
		c = read_line(fd, line);
		append(result, line);
	} while(c != EOF);
	
	return result;
}

static char read_line(int fd, char *buffer) {
	//for condizioni uscita per linea troppo lunga o linea finita
}

