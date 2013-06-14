/** @file
	Code for the main process, 
	which performs simulation setup and execution management.
*/

#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "io_utils.h"
#include "ipc_utils.h"
#include "list.h"
#include "operation.h"

/// Typedef for signal handling
typedef void (*sighandler_t)(int);

/**
	IDs for the used IPCs
	<ul><li>Cell 0: semaphore set
	<li>Cell 1: shared memory for operations
	<li>Cell 2: shared memory for processor states</ul>
*/
int ipc_id[] = {-1, -1, -1};

/** The number of processors */
int processors;

static int find_proc(int *states);
static list* parse_file(const char *const pathname);
static void start_processors(void);
static void stop_execution(int signal);

/**
	Main function
	@param argc The number of arguments
	@param argv The array of arguments
*/
int main(int argc, char *argv[]) {
	int *results, *shm_states;
	int i, op_count, proc_id;
	char *tmp_operator, *cmd;
	list *commands;
	operation *shm_operations;
	
	if(signal(SIGTERM, &stop_execution) == SIG_ERR) {
		perror("Failed to register signal");
		exit(1);
	}
	if(argc != 3) {
		write_to_fd(2, "Usage: main.x <source file> <results file>\n");
		exit(1);
	}
	commands = parse_file(argv[1]);
	processors = atoi(list_extract(commands));
	if (processors == 0) {
		write_to_fd(2, "Invalid number of processors\n");
		exit(1);
	}
	op_count = list_count(commands);
	if (op_count == 0) {
		write_to_fd(2, "No operations provided\n");
		exit(1);
	}
	write_with_int(1, "Main - Number of operations: ", op_count);		
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
		cmd = list_extract(commands);
		proc_id = atoi(strtok(cmd, " "));
		sem_p(2 * processors + 1);
		if (proc_id-- == 0) {
			proc_id = find_proc(shm_states);
		}
		sem_p(2 * proc_id);
		if (shm_states[proc_id]++ != 0)
			results[shm_states[proc_id] * -1] = shm_operations[proc_id].num1;
		shm_operations[proc_id].num1 = atoi(strtok(NULL, " "));
		tmp_operator = strtok(NULL, " ");
		shm_operations[proc_id].op = *tmp_operator;
		shm_operations[proc_id].num2 = atoi(strtok(NULL, " "));
		shm_states[proc_id] = i;
		sem_v((2 * proc_id) + 1);
		free(cmd);
	}
	
	list_destruct(commands);
	
	for (i = 0; i < processors; ++i) {
		sem_p(2 * i);
		if (shm_states[i]++ != 0)
			results[shm_states[i] * -1] = shm_operations[i].num1;
		shm_operations[i].op = 'K';
		sem_v((2 * i) + 1);
	}

	for (i = 0; i < processors; ++i) 
		if(wait(NULL) == -1)
			perror("Wait failed");

	write_results(argv[2], results, op_count);
	close_ipc();
	exit(0);
}

static int find_proc(int *states) {
	int i = 0;

	sem_p(2 * processors);
	while(states[i++] > 0);
	sem_v(2 * processors);
	return i - 1;
}

static list* parse_file(const char *const pathname) {
	list *result = list_construct();
	char line[50];
	int len, fd;
	
	if(result == NULL) 
		exit(1);

	fd = open(pathname, O_RDONLY);
	if(fd == -1) {
		perror("Failed to open setup file");
		exit(1);
	}
		
	do {
		len = read_line(fd, line, 50);
		if (len > 0)
			list_append(result, line);
	} while(len >= 0);
	
	if (close(fd) == -1) {
		perror("Failed to close setup file");
		exit(1);
	}

	return result;
}

static void start_processors(void) {
	int i, pid;
	char proc_id[8], nsems[8];

	if(itoa((2 * processors) + 2, nsems, 8) == -1) {
		write_to_fd(2, "Failed to convert number of semaphores\n");	
		kill(0, SIGTERM);
	}
	for (i = 0; i < processors; ++i) {
		pid = fork();
		if (pid == -1) {
			perror("Failed to fork processor");
			kill(0, SIGTERM);
		}
		if (pid == 0) {
			if(itoa(i, proc_id, 8) != -1)
				execl("processor.x", "processor.x", proc_id, nsems, (char *) NULL);
			kill(0, SIGTERM);
		}
	}
}

static void stop_execution(int signum) {
	close_ipc();
	exit(1);
}
