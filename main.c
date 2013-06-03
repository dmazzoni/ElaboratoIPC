#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include "lib/io_utils.h"
#include "lib/ipc_utils.h"
#include "lib/list.h"
#include "lib/operation.h"

static list* commands_setup(char *const pathname);

int main(int argc, char *argv[]) {
	int semkey, shmkey1, shmkey2, semid, shmid1, shmid2;
	int processors;
	list *commands;
	
	if(argc != 2) {
		write_err("Usage: main.x <pathname>\n");
		exit(1);
	}
	
	commands = commands_setup(argv[1]);
		
	processors = atoi(list_extract(commands));
	if (processors == 0) {
		write_err("Invalid number of processors\n");
		exit(1);
	}
		
	semkey = generate_key(1);
	shmkey1 = generate_key(2);
	shmkey2 = generate_key(3);
	
	/*
	k operazioni su n figli
	- 2n + 1 semafori per interazione sulle operazioni
	- 1 semaforo per SHM2
	- SHM 1 --> Spazio per n struct operation
	- SHM 2 --> Vettore stati figli (+ contatore)
	Locali al padre:
	- Vettore con spazio per k risultati
	*/
}

static list* commands_setup(char *const pathname) {
	int fd;
	list* result;
	
	fd = open(pathname, O_RDONLY);
	if(fd == -1) {
		perror("Failed to open setup file");
		exit(1);
	}

	result = parse_file(fd);
	if (result == NULL) {
		write_err("Failed to create command list\n");
		exit(1);
	}
	
	if (close(fd) == -1) {
		perror("Failed to close setup file");
		exit(1);
	}
	return result;
}
