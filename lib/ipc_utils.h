#ifndef IPC_UTILS_H
#define IPC_UTILS_H

void init_ipc(int nsems, int shm1_size, int shm2_size, int flags);
int close_ipc(void);
void init_sems(int processors);
void sem_p(int semnum);
void sem_v(int semnum);
void* shm_attach(int shm_id);
void shm_detach(void *address);
void kill_group(int signum);

#endif
