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
    int shmid;
    void *page_start;
    int *p;
    int i, value;

    int  buf_out = 0; /*从缓冲区读取位置*/
    empty = sem_open("empty", BUFSIZE);
    full = sem_open("full", 0);
    mutex = sem_open("mutex", 1);
    if (empty==SEM_FAILED || full==SEM_FAILED || mutex==SEM_FAILED) {
      perror("sem_open() failed!\n");
      return -1;
    }

    /* shmid = shmget((key_t)1, 4096, 0666|IPC_CREAT); */
    shmid = shmget((key_t)1, 4096);
    printf("consumer shmid: %d\n", shmid);
    fflush(stdout);
    if (shmid == -1) {
        perror("shmget() failed!\n");
        return -1;
    }

    /*  page_start = shmat(shmid, 0, 0); */
    page_start = shmat(shmid);
    if (page_start == (void*)-1) {
        perror("shmat() failed!\n");
        return -1;
    }
    p = (int*) page_start;

    for (i=0; i<NUMBER; i++) {
        sem_wait(full);
        sem_wait(mutex);
        value = p[buf_out];
        buf_out = (buf_out + 1) % BUFSIZE;
        sem_post(mutex);
        sem_post(empty);

        printf("%d\n", value);
        fflush(stdout);
    }

    sem_unlink("full");
    sem_unlink("empty");
    sem_unlink("mutex");

    return 0;
}