#include <stdio.h>

int ipc_id[] = {-1, -1, -1};

int main(int argc, char *argv[]) {
	int i;
	
	for (i = 0; i < argc; i++) {
		printf("%s\t", argv[i]);
		fflush(stdout);
	}
	putchar('\n');
	while(1);
}
