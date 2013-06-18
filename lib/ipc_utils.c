/** @file
	Contains functions for IPC initialization and management.
*/

#include <signal.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/types.h>
#include "io_utils.h"
#include "ipc_utils.h"

extern int ipc_id[];

static void sem_operation(int semnum, short op);
static int generate_key(int seed);

/// Used for calls to @c semctl()
union semun {
	int val;
	struct semid_ds *buf;
	unsigned short *array;
	struct seminfo *__buf;
};

/**
	Creates the semaphore set and the shared memory segments, 
	saving their IDs into @c ipc_id
	Wraps the @c semget() and @c shmget() system calls, checking their exit code.
	@param nsems The number of semaphores
	@param shm1_size The size of the shared memory for operations
	@param shm2_size The size of the shared memory for processor states
	@see ipc_id
*/
void init_ipc(int nsems, int shm1_size, int shm2_size, int flags) {

	ipc_id[0] = semget(generate_key(1), nsems, flags);
	if(ipc_id[0] == -1) {
		write_to_fd(2, "Failed to create semaphore set");
		kill(0, SIGTERM);
	}

	ipc_id[1] = shmget(generate_key(2), shm1_size, flags);
	if(ipc_id[1] == -1) {
		write_to_fd(2, "Failed to create shared memory for operations");
		kill(0, SIGTERM);
	}

	ipc_id[2] = shmget(generate_key(3), shm2_size, flags);
	if(ipc_id[2] == -1) {
		write_to_fd(2, "Failed to create shared memory for processor states");
		kill(0, SIGTERM);
	}
}

/**
	Closes the semaphore set and the shared memory segments.<br>
	Wraps the @c semctl() and @c shmctl() system calls, checking their exit code.
	@return 0 if all IPCs are successfully closed, -1 otherwise.
*/
int close_ipc(void) {
	int i, ret = 0;
	
	if(semctl(ipc_id[0], 0, IPC_RMID, NULL) == -1)  
		ret = -1;
	for(i = 1; i < 3; ++i)
		if(shmctl(ipc_id[i], IPC_RMID, NULL) == -1)
			ret = -1;
	return ret;
}

/**
	Initializes the semaphore set as follows:<ul>
	<li>Semaphores used by the main process to wait for a specific 
	processor are set to 1.
	<li>Semaphores used by each processor to wait for an operation are set to 0.
	<li>The semaphore which enforces mutually exclusive access to the
	shared memory for processor states is set to 1.
	<li>The semaphore used by the main process to wait when no
	processors are available is set to @c processors.
	</ul>Wraps the @c semctl() system call, checking its exit code.
	@param processors The number of processors
*/
void init_sems(int processors) {
	int i, nsems = 2 * processors + 2;
	union semun args;
	unsigned short values[nsems];
	
	for (i = 0; i < processors; ++i) {
		values[2 * i] = 1;
		values[(2 * i) + 1] = 0;
	}
	values[nsems - 2] = 1;
	values[nsems - 1] = processors;
	args.array = values;
	if (semctl(ipc_id[0], -1, SETALL, args) == -1) {
		write_to_fd(2, "Failed to initialize semaphore set");
		kill(0, SIGTERM);
	}
}

/**
	Implements the @c wait operation on the specified semaphore.
	@param semnum The semaphore to wait
	@see sem_operation
*/
void sem_p(int semnum) {
	sem_operation(semnum, -1);
}

/**
	Implements the @c signal operation on the specified semaphore.
	@param semnum The semaphore to signal
	@see sem_operation
*/
void sem_v(int semnum) {
	sem_operation(semnum, 1);
}

/**
	Wraps the @c shmat() system call, checking its exit code.
	@param shm_id The shared memory segment's ID
	@return A pointer to the attached segment.
*/
void* shm_attach(int shm_id) {
	void *ret = shmat(shm_id, NULL, 0);
	if (ret == (void *) -1) {
		write_to_fd(2, "Failed to attach shared memory segment");
		kill(0, SIGTERM);
	}
	return ret;
}

/**
	Wraps the @c shmdt() system call, checking its exit code.
	@param address The attached address to detach
*/
void shm_detach(void *address) {
	if (shmdt(address) == -1)
		write_to_fd(2, "Failed to detach shared memory segment");
}

/**
	Wraps the @c ftok() system call, checking its exit code.
	@param seed The seed to generate the key
*/
static int generate_key(int seed) {
	int key = ftok("main.c", seed);
	if (key == -1) {
		write_to_fd(2, "ftok failed");
		exit(1);
	}
	return key;
}

/**
	Performs the specified operation on the given semaphore.<br>
	Wraps the @c semop() system call, checking its error code.
	@param semnum The semaphore number
	@param op The operation to perform
*/
static void sem_operation(int semnum, short op) {
	struct sembuf sops;
	
	sops.sem_num = semnum;
	sops.sem_op = op;
	sops.sem_flg = 0;
	if (semop(ipc_id[0], &sops, 1) == -1) {
		write_to_fd(2, "Failed semaphore operation");
		kill(0, SIGTERM);
	}
}
