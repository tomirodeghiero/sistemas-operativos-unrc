#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>

int main()
{
	// ftok to generate unique key
	key_t key = ftok("shmfile", 65);
    if (key == -1) {
        perror("ftok");
        return 1;
    }

	// shmget returns an identifier in shmid
	int shmid = shmget(key, 1024, 0666 | IPC_CREAT);
    if (shmid == -1) {
        perror("shmget");
        return 1;
    }


	// shmat to attach to shared memory
	char *str = (char*) shmat(shmid, (void*)0, 0);
    if (str == (char*)-1) {
        perror("shmat");
        return 1;
    }

    printf("shared memory logical address: %p\n", str);
	printf("Data read from memory: %s\n", str);
	
	//detach from shared memory
	if (shmdt(str) == -1) {
        perror("shmdt");
        return 1;
    }
	
	// destroy the shared memory segment
	// shmctl(shmid, IPC_RMID, NULL);
    // (comentado para el inciso b)
	
	return 0;
}
