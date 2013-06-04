#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include "lib/io_utils.h"
#include "lib/ipc_utils.h"
#include "lib/list.h"
#include "lib/operation.h"

int main(int argc, char *argv[]) {
	int ipc_id[] = {-1, -1, -1};
	int *results;
	int processors, op_count;
	list *commands;
	
	if(argc != 2) {
		write_err("Usage: main.x <pathname>\n");
		exit(1);
	}
	commands = parse_file(argv[1]);
	processors = atoi(list_extract(commands));
	if (processors == 0) {
		write_err("Invalid number of processors\n");
		exit(1);
	}
	op_count = list_count(commands);
	if (op_count == 0) {
		write_err("No operations provided\n");
		exit(1);
	}		
	results = (int *) malloc(op_count * sizeof(int));
	if (results == NULL) {
		perror("Failed to allocate results array");
		exit(1);	
	}
	init_ipc(ipc_id, (2 * processors) + 1, processors * sizeof(operation), (processors + 1) * sizeof(int), 0666 | IPC_CREAT | IPC_EXCL);	
		
	
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
