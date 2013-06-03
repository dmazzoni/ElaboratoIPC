#include <fcntl.h> 
#include <unistd.h>
#include <stdlib.h>
#include "lib/io_utils.h"
#include "lib/list.h"
#include "lib/operation.h"

int main(int argc, char *argv[]) {
	int fd;
	list *commands;
	
	if(argc != 2)
		write_err("Usage: main.x <pathname>\n");
		
	fd = open(argv[1], O_RDONLY);
	if(fd == -1) {
		perror("Failed to open setup file");
		exit(1);
	}
	
	commands = parse_file(fd);
	if (commands == NULL)
		write_err("Failed to create command list\n");
	
}
