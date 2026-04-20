#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <string.h>
  
int main()
{
    // ftok to generate unique key
    key_t key = ftok("shmfile", 65);
    if (key == -1) {
        perror("ftok");
        return 1;
    }
  
    // shmget returns an identifier in shmid
    int shmid = shmget(key, 1024, 0666|IPC_CREAT);
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
  
    printf("Data to write to shmem: ");
    if (fgets(str, 1024, stdin) == NULL) {
        perror("fgets");
        shmdt(str);
        return 1;
    }
    str[strcspn(str, "\n")] = '\0';
  
    printf("Data written in memory: %s\n", str);
      
    //detach from shared memory 
    if (shmdt(str) == -1) {
        perror("shmdt");
        return 1;
    }
  
    return 0;
}
