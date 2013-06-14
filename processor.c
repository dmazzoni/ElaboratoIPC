#include <signal.h>
#include <stdlib.h>
#include <sys/types.h>
#include "io_utils.h"
#include "ipc_utils.h"
#include "operation.h"

int ipc_id[] = {-1, -1, -1};

static void compute(operation *oper);

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
		compute(shm_operations + proc_id);
		sem_p(nsems - 2);
		shm_states[proc_id] *= -1;
		sem_v(nsems - 2);
		sem_v(2 * proc_id);
		sem_v(nsems - 1);	
	}
}

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
		case 'K': exit(0);
		default: write_to_fd(2, "Invalid operator\n"); 
			 kill(0, SIGTERM); 
	}
}
