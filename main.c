#include <signal.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <unistd.h>
#include "lib/io_utils.h"
#include "lib/ipc_utils.h"
#include "lib/list.h"
#include "lib/operation.h"

typedef void (*sighandler_t)(int);
int ipc_id[] = {-1, -1, -1};
int processors;

static void start_processors(void);
static void stop_execution(int signal);

int main(int argc, char *argv[]) {
	int *results, *shm_states;
	int i, op_count, proc_id;
	char *tmp_operator;
	list *commands;
	operation *shm_operations;
	
	if(signal(SIGTERM, &stop_execution) == SIG_ERR) {
		perror("Failed to register signal");
		exit(1);
	}
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
	
	init_ipc(2 * processors + 2, processors * sizeof(operation), processors * sizeof(int), 0666 | IPC_CREAT | IPC_EXCL);
	init_sems(processors);
	shm_operations = (operation *) shm_attach(ipc_id[1]);
	shm_states = (int *) shm_attach(ipc_id[2]);
	
	for (i = 0; i < processors; ++i)
		shm_states[i] = 0;
	
	start_processors();
	for (i = 1; list_count(commands) > 0; ++i) {
		proc_id = atoi(strtok(list_extract(commands), " "));
		sem_p(2 * processors + 1);
		if (proc_id-- == 0) {
			//proc_id = find_proc();
		}
		sem_p(proc_id);
		if (shm_states[proc_id]++ != 0)
			results[shm_states[proc_id] * -1] = shm_operations[proc_id].num1;
		shm_operations[proc_id].num1 = atoi(strtok(NULL, " ");
		tmp_operator = strtok(NULL, " ");
		shm_operations[proc_id].op = *tmp_operator;
		shm_operations[proc_id].num2 = atoi(strtok(NULL, " ");
		shm_states[proc_id] = i;
		sem_v(processors + proc_id);
	}
}

static void start_processors(void) {
	int i, pid;
	char buffer[8];
	
	for (i = 0; i < processors; ++i) {
		pid = fork();
		if (pid == -1) {
			perror("Failed to fork processor");
			kill(0, SIGTERM);
		}
		if (pid == 0) {
			if(itoa(i, buffer, 8) != -1)
				execl("processor.x", "processor.x", buffer, (char *) NULL);
			kill(0, SIGTERM);
		}
	}
}

static void stop_execution(int signum) {
	close_ipc();
	exit(1);
}
