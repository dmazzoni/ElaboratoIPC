/** @file
	Code for the child processor, which is
	started by the main process.<br>
	Each processor is launched passing its ID and the
	total number of semaphores: with this information the
	processor can synchronize on the correct semaphore.<br>
	After attaching the shared memory segments, it loops
	while there are operations to compute, and writes the 
	results in the operations array.
*/

#include <signal.h> 
#include <stdlib.h>
#include <sys/types.h>
#include "io_utils.h"
#include "ipc_utils.h"
#include "operation.h"

/**
	IDs for the used IPCs
	<ul><li>Cell 0: semaphore set
	<li>Cell 1: shared memory for operations
	<li>Cell 2: shared memory for processor states</ul>
*/
int ipc_id[] = {-1, -1, -1};

static void compute(operation *oper);

/**
	Computes the operations while they are provided by 
	the main processor and returns the results in the
	corresponding shared memory locations, with synchronized access.
	@param argc The number of arguments
	@param argv The array of arguments
*/
int main(int argc, char *argv[]) {
	int proc_id, nsems;
	int *shm_states;
	operation *shm_operations;
		
	proc_id = atoi(argv[1]);
	nsems = atoi(argv[2]);
	init_ipc(0, 0, 0, 0666);
	shm_operations = (operation *) shm_attach(ipc_id[1]);
	shm_states = (int *) shm_attach(ipc_id[2]);

	while(1) {
		sem_p((2 * proc_id) + 1);
		if (shm_operations[proc_id].op == 'K')
			break;
		compute(shm_operations + proc_id);
		sem_p(nsems - 2);
		shm_states[proc_id] *= -1;
		sem_v(nsems - 2);
		sem_v(2 * proc_id);
		sem_v(nsems - 1);	
	}
	
	shm_detach((void *) shm_operations);
	shm_detach((void *) shm_states);
	exit(0);
}

/**
	Calculates the operation passed and stores the result in
	the first operand field.
	@param oper The operation to execute
*/
static void compute(operation *oper) {
	switch(oper->op) {
		case '+': oper->num1 = oper->num1 + oper->num2; break;
		case '-': oper->num1 = oper->num1 - oper->num2; break;
		case '*': oper->num1 = oper->num1 * oper->num2; break;
		case '/': if(oper->num2 != 0)
				oper->num1 = oper->num1 / oper->num2; 
			  else {
				write_to_fd(2, "Division by 0\n"); 
				kill(0, SIGTERM);
			  }; break;
		default: write_to_fd(2, "Invalid operator\n"); 
			 kill(0, SIGTERM); 
	}
}
