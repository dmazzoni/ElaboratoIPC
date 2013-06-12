#include <signal.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/types.h>
#include "ipc_utils.h"

extern int ipc_id[];

static void sem_operation(int semnum, short op);
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
		perror("Failed to create semaphore set");
		kill(0, SIGTERM);
	}

	ipc_id[1] = shmget(generate_key(2), shm1_size, flags);
	if(ipc_id[1] == -1) {
		perror("Failed to create shared memory for operations");
		kill(0, SIGTERM);
	}

	ipc_id[2] = shmget(generate_key(3), shm2_size, flags);
	if(ipc_id[2] == -1) {
		perror("Failed to create shared memory for processor states");
		kill(0, SIGTERM);
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

void init_sems(int processors) {
	int i, nsems = 2 * processors + 2;
	union semun args;
	unsigned short values[nsems];
	
	for (i = 0; i < processors; ++i) {
		values[i] = 1;
		values[i + processors] = 0;
	}
	values[nsems - 2] = 1;
	values[nsems - 1] = processors;
	args.array = values;
	if (semctl(ipc_id[0], -1, SETALL, args) == -1) {
		perror("Failed to initialize semaphore set");
		kill(0, SIGTERM);
	}
}

void sem_p(int semnum) {
	sem_operation(semnum, -1);
}

void sem_v(int semnum) {
	sem_operation(semnum, 1);
}

void* shm_attach(int shm_id) {
	void *ret = shmat(shm_id, NULL, 0);
	if (ret == (void *) -1) {
		perror("Failed to attach shared memory segment");
		kill(0, SIGTERM);
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

static void sem_operation(int semnum, short op) {
	struct sembuf sops;
	
	sops.sem_num = semnum;
	sops.sem_op = op;
	sops.sem_flg = 0;
	if (semop(ipc_id[0], &sops, 1) == -1) {
		perror("Failed semaphore operation");
		kill(0, SIGTERM);
	}
}
