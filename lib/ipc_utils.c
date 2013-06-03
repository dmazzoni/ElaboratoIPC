#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/types.h>

int generate_key(int seed) {
	int key = ftok("main.c", seed);
	if (key == -1) {
		perror("ftok failed");
		exit(1);
	}
	return key;
}

