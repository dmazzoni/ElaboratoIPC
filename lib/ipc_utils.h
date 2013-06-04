#ifndef IPC_UTILS_H
#define IPC_UTILS_H

void init_ipc(int *ipc_id, int nsems, int shm1_size, int shm2_size, int flags);
int close_ipc(int *ipc_id);

#endif
