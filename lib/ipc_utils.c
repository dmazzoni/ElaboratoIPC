#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/types.h>

void init_ipc(int *ipc_id, int nsems, int shm1_size, int shm2_size, int flags) {
	int i;

	ipc_id[0] = semget(generate_key(1), nsems, flags);
	if(ipc_id[0] == -1) {
		close_ipc(ipc_id);
		exit(1);
	}
	
	// Inizializzazione semafori in funzione separata	

	ipc_id[1] = shmget(generate_key(2), shm1_size, flags);
	if(ipc_id[1] == -1) {
		close_ipc(ipc_id);
		exit(1);
	}

	ipc_id[2] = shmget(generate_key(3), shm2_size, flags);
	if(ipc_id[2] == -1) {
		close_ipc(ipc_id);
		exit(1);
	}
}

int close_ipc(int *ipc_id) {
	int i, ret = 0;
	
	if(semctl(ipc_id[0], 0, IPC_RMID, NULL) == -1)  
		ret = -1;
	for(i = 1, i < 3; ++i)
		if(shmctl(ipc_id[i], IPC_RMID, NULL) == -1)
			ret = -1;
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


