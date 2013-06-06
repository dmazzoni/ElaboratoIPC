#include <stdlib.h>
#include <sys/ipc.h>
#include <unistd.h>
#include "lib/io_utils.h"
#include "lib/ipc_utils.h"
#include "lib/list.h"
#include "lib/operation.h"

int ipc_id[] = {-1, -1, -1};
int processors, nsems;
int *processor_pids;

static int* init_processors(void);

int main(int argc, char *argv[]) {
	int *results, *shm_states, *shm_busy_count;
	int op_count;
	list *commands;
	operation *shm_operations;
	
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
	
	nsems = (2 * processors) + 2;
	init_ipc(nsems, processors * sizeof(operation), (processors + 1) * sizeof(int), 0666 | IPC_CREAT | IPC_EXCL);
	init_sems();
	shm_operations = (operation *) shm_attach(ipc_id[1]);
	shm_busy_count = (int *) shm_attach(ipc_id[2]);
	shm_states = shm_busy_count + 1;
	
	processor_pids = init_processors();
	
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

static int* init_processors(void) {
	int i;
	int *pids = (int *) malloc(processors * sizeof(int));
	char buffer[8];
	
	if (pids == NULL) {
		perror("Failed to allocate processor PIDs array");
		close_ipc();
		exit(1);
	}
	for (i = 0; i < processors; ++i)
		pids[i] = 0;
	for (i = 0; i < processors; ++i) {
		pids[i] = fork();
		if (pids[i] == -1) {
			perror("Failed to fork processor");
			/*kill_processors(); //TODO
			close_ipc();
			exit(1);*/
		}
		if (pids[i] == 0) {
			itoa(i, buffer, 10);
			execl("processor.x", "processor.x", buffer, (char *) NULL);
			// Stessa gestione sopra
		}
	}
}
