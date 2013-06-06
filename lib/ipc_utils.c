#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/types.h>
#include "io_utils.h"
#include "ipc_utils.h"

extern int ipc_id[], nsems;

static int generate_key(int seed);

union semun {
	int val;
	struct semid_ds *buf;
	unsigned short *array;
	struct seminfo *__buf;
};

void init_ipc(int nsems, int shm1_size, int shm2_size, int flags) {
	int i;

	ipc_id[0] = semget(generate_key(1), nsems, flags);
	if(ipc_id[0] == -1) {
		close_ipc();
		write_err("Failed to create semaphore set\n");
		exit(1);
	}

	ipc_id[1] = shmget(generate_key(2), shm1_size, flags);
	if(ipc_id[1] == -1) {
		close_ipc();
		write_err("Failed to create shared memory for operations\n");
		exit(1);
	}

	ipc_id[2] = shmget(generate_key(3), shm2_size, flags);
	if(ipc_id[2] == -1) {
		close_ipc();
		write_err("Failed to create shared memory for processor states\n");
		exit(1);
	}
}

int close_ipc(void) {
	int i, ret = 0;
	
	if(semctl(ipc_id[0], 0, IPC_RMID, NULL) == -1)  
		ret = -1;
	for(i = 1; i < 3; ++i)
		if(shmctl(ipc_id[i], IPC_RMID, NULL) == -1)
			ret = -1;
	return ret;
}

void init_sems(void) {
	int i;
	union semun args;
	unsigned short values[nsems];
	
	for (i = 0; i < nsems; ++i)
		values[i] = 0;
	args.array = values;
	if (semctl(ipc_id[0], -1, SETALL, args) == -1) {
		perror("Failed to initialize semaphore set");
		exit(1);
	}
}

void* shm_attach(int shm_id) {
	void *ret = shmat(shm_id, NULL, 0);
	if (ret == (void *) -1) {
		perror("Failed to attach shared memory segment");
		exit(1);
	}
	return ret;
}

static int generate_key(int seed) {
	int key = ftok("main.c", seed);
	if (key == -1) {
		perror("ftok failed");
		exit(1);
	}
	return key;
}
