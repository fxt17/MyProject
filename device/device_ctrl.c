#include <stdio.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <string.h>

#define SHM_SIZE 1024

int main() {
    key_t key = ftok("/tmp", 'A');
    int shmid = shmget(key, SHM_SIZE, IPC_CREAT | 0666);// 创建/获取共享内存段
    
    if (shmid == -1) {
        perror("shmget");
        return 1;
    }
    
    char *shm_ptr = (char*)shmat(shmid, NULL, 0);// 附加到进程地址空间
    if (shm_ptr == (void*)-1) {
        perror("shmat");
        return 1;
    }
    
    // 写入数据
    strcpy(shm_ptr, "Hello, Shared Memory!");
    
    // 读取数据（在另一个进程中）
    // printf("Read from shared memory: %s\n", shm_ptr);
    
    shmdt(shm_ptr);
    shmctl(shmid, IPC_RMID, NULL); // 删除共享内存
    return 0;
}