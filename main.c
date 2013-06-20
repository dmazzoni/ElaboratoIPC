/** @file
	Code for the main process, which performs 
	simulation setup and execution management.<br>
	After setting up IPCs and data structures, 
	the main process does the following:<ul>
	<li>Executes the required number of processors
	<li>Dispatches each operation to the appropriate processor,
	collecting the latest computed result
	<li>Writes the results on the specified output file</ul>
*/

#include <fcntl.h>
#include <signal.h>
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
	Carries out simulation setup and management.
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
		write_to_fd(2, "Failed to register signal\n");
		exit(1);
	}
	if(argc != 3) {
		write_to_fd(2, "Usage: main.x <source file> <results file>\n");
		exit(1);
	}
	commands = parse_file(argv[1]);
	processors = atoi(list_extract(commands));
	if (processors <= 0) {
		write_to_fd(2, "Invalid number of processors\n");
		exit(1);
	}
	write_with_int(1, "Number of processors: ", processors);
	op_count = list_count(commands);
	if (op_count == 0) {
		write_to_fd(2, "No operations provided\n");
		exit(1);
	}
	write_with_int(1, "Number of operations: ", op_count);		
	results = (int *) malloc(op_count * sizeof(int));
	if (results == NULL) {
		write_to_fd(2, "Failed to allocate results array\n");
		exit(1);	
	}
	
	init_ipc(2 * processors + 2, processors * sizeof(operation), processors * sizeof(int), 0666 | IPC_CREAT | IPC_EXCL);
	write_with_int(1, "Created semaphore set with ID ", ipc_id[0]);
	write_with_int(1, "Created shm for operations with ID ", ipc_id[1]);
	write_with_int(1, "Created shm for states with ID ", ipc_id[2]);
	init_sems(processors);
	shm_operations = (operation *) shm_attach(ipc_id[1]);
	shm_states = (int *) shm_attach(ipc_id[2]);
	
	for (i = 0; i < processors; ++i)
		shm_states[i] = 0;
	
	start_processors();
	for (i = 1; list_count(commands) > 0; ++i) {
		cmd = list_extract(commands);
		write_with_int(1, "\nOperation #", i);
		proc_id = atoi(strtok(cmd, " "));
		sem_p(2 * processors + 1);
		if (proc_id-- == 0) {
			proc_id = find_proc(shm_states);
		}
		write_with_int(1, "Waiting for processor ", proc_id + 1);
		sem_p(2 * proc_id);
		write_with_int(1, "Delivering operation to processor ", proc_id + 1);
		if (shm_states[proc_id]++ != 0) {
			results[shm_states[proc_id] * -1] = shm_operations[proc_id].num1;
			write_with_int(1, "Previous result: ", shm_operations[proc_id].num1);
		}
		shm_operations[proc_id].num1 = atoi(strtok(NULL, " "));
		tmp_operator = strtok(NULL, " ");
		shm_operations[proc_id].op = *tmp_operator;
		shm_operations[proc_id].num2 = atoi(strtok(NULL, " "));
		shm_states[proc_id] = i;
		write_with_int(1, "Operation delivered. Unblocking processor ", proc_id + 1);
		sem_v((2 * proc_id) + 1);
		free(cmd);
	}
	
	list_destruct(commands);
	
	for (i = 0; i < processors; ++i) {
		sem_p(2 * i);
		write_with_int(1, "\nPassing termination command to processor #", i + 1);
		if (shm_states[i]++ != 0) {
			results[shm_states[i] * -1] = shm_operations[i].num1;
			write_with_int(1, "Last result: ", shm_operations[i].num1);
		}
		shm_operations[i].op = 'K';
		sem_v((2 * i) + 1);
	}

	for (i = 0; i < processors; ++i) 
		if(wait(NULL) == -1)
			write_to_fd(2, "Wait failed\n");
			
	write_to_fd(1, "\nAll processors exited. Writing output file\n");
	write_results(argv[2], results, op_count);
	write_to_fd(1, "Closing IPCs\n");
	shm_detach((void *) shm_operations);
	shm_detach((void *) shm_states);
	close_ipc();
	exit(0);
}

/**
	Searches the processor state array for a free processor (state <= 0).
	@param states The array of processor states. Each cell <i>i</i> contains a 
	key such that:<br>
	<ul><li>If <b>key < 0</b>, processor has completed the |key|-th operation
	<li>If <b>key == 0</b>, processor has not received an operation yet
	<li>If <b>key > 0</b>, processor is currently working on the key-th operation</ul>
	@return The ID of a free processor
*/
static int find_proc(int *states) {
	int i = 0;

	sem_p(2 * processors);
	write_to_fd(1, "Looking for a free processor\n");
	while(states[i++] > 0);
	sem_v(2 * processors);
	write_with_int(1, "Found processor ", i);
	return i - 1;
}

/**
	Reads the specified setup file, 
	building a list of strings containing the operations to simulate.<br>
	Assumes that the file has a correct structure.
	@param pathname The setup file's path
	@return The list of operations to compute
*/
static list* parse_file(const char *const pathname) {
	list *result = list_construct();
	char line[50];
	int len, fd;
	
	if(result == NULL) 
		exit(1);

	fd = open(pathname, O_RDONLY);
	if(fd == -1) {
		write_to_fd(2, "Failed to open setup file\n");
		exit(1);
	}
		
	do {
		len = read_line(fd, line, 50);
		if (len > 0)
			list_append(result, line);
	} while(len >= 0);
	
	if (close(fd) == -1) {
		write_to_fd(2, "Failed to close setup file\n");
		exit(1);
	}

	return result;
}

/**
	Forks and executes the required number of processors.
*/
static void start_processors(void) {
	int i, pid;
	char proc_id[8], nsems[8];

	if(itoa((2 * processors) + 2, nsems, 8) == -1) {
		write_to_fd(2, "Failed to convert number of semaphores\n");	
		kill_group(SIGTERM);
	}
	for (i = 0; i < processors; ++i) {
		pid = fork();
		if (pid == -1) {
			write_to_fd(2, "Failed to fork processor\n");
			kill_group(SIGTERM);
		}
		if (pid == 0) {
			if(itoa(i, proc_id, 8) != -1)
				execl("processor.x", "processor.x", proc_id, nsems, (char *) NULL);
			kill_group(SIGTERM);
		}
	}
}

/**
	Handles the @c SIGTERM signal by closing all IPCs.
	@param signum The received signal
*/
static void stop_execution(int signum) {
	write_to_fd(2, "SIGTERM received. Closing IPCs\n");
	close_ipc();
	exit(1);
}
