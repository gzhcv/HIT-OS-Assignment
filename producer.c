#define __LIBRARY__
#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<errno.h>

_syscall2(sem_t*,sem_open,const char *,name,unsigned int,value);
_syscall1(int,sem_wait,sem_t*,sem);
_syscall1(int,sem_post,sem_t*,sem);
_syscall1(int,sem_unlink,const char *,name);
_syscall2(int, shmget, key_t, key, size_t, size);
_syscall1(void*, shmat, int, shmid);

#define NUMBER 10
#define BUFSIZE 5

sem_t *empty, *full, *mutex;

int main()
{

    int i, shmid;
    void* page_start;
    int *p;
    int  buf_in = 0; /*写入缓冲区位置*/

    empty = sem_open("empty", BUFSIZE);
    full = sem_open("full", 0);
    mutex = sem_open("mutex", 1);
    if (empty==SEM_FAILED || full==SEM_FAILED || mutex==SEM_FAILED) {
      perror("sem_open() failed!\n");
      return -1;
    }

    /* shmid = shmget((key_t)1, 4096, 0666|IPC_CREAT); */
    shmid = shmget((key_t)1, 4096);
    printf("producer shmid: %d\n", shmid);
    if (shmid == -1) {
        perror("shmget() failed!\n");
        return -1;
    }

    /* page_start = shmat(shmid, 0, 0); */
    page_start = shmat(shmid);
    if (page_start == (void*)-1) {
        perror("shmat() failed!\n");
        return -1;
    }

    p = (int*) page_start;

    for (i=0; i<NUMBER; i++) {
        sem_wait(empty);
        sem_wait(mutex);
        p[buf_in] = i;
        buf_in = (buf_in + 1) % BUFSIZE;
        sem_post(mutex);
        sem_post(full);

        printf("write in buffer %d\n", i);
        fflush(stdout);
    }

    /* 由于producer进程结束后操作系统会回收该进程页表中对应的物理内存，包括共享内存，所以此处sleep一下
       解决办法是新的进程使用共享内存时，mem_map数组对应的物理页元素值+1，实现中...
    */
    sleep(20);

    sem_unlink("full");
    sem_unlink("empty");
    sem_unlink("mutex");
    return 0;
}