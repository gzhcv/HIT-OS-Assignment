#define __LIBRARY__
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <stdlib.h>

_syscall1(int, sem_wait, sem_t*, sem);
_syscall1(int, sem_post, sem_t*, sem);
_syscall1(int, sem_unlink, const char*, name);
_syscall2(sem_t*, sem_open, const char*, name, unsigned int, value);

#define NUMBER 500
#define CHILD 5
#define BUFSIZE 10

sem_t *empty, *full, *mutex;

int main()
{
    int i,j,k;
    int data;
    int fd;
    pid_t p;
    int  buf_out = 0; /*从缓冲区读取位置*/
    int  buf_in = 0; /*写入缓冲区位置*/

    empty = sem_open("empty", 10);
    full = sem_open("full", 0);
    mutex = sem_open("mutex", 1);

    if (empty==SEM_FAILED || full==SEM_FAILED || mutex==SEM_FAILED) {
      perror("sem_open() failed!\n");
      return -1;
    }

    fd = open("buffer.dat",O_CREAT|O_RDWR|O_TRUNC,0666);
    lseek(fd, 10*sizeof(int), SEEK_SET);
    write(fd, (char *)&buf_out, sizeof(int));

    if (!(p=fork())) {
        for (i=0; i<NUMBER; i++) {
            sem_wait(empty);
            sem_wait(mutex);

            lseek(fd, buf_in*sizeof(int), SEEK_SET);
            write(fd, (char *)&i, sizeof(int));
            buf_in = (buf_in + 1)% BUFSIZE;

            sem_post(mutex);
            sem_post(full);
        }
        return 0;
    }

    if (p < 0) {
        perror("failed to fork producer process!\n");
        return -1;
    }

    for (j=0; j<CHILD; j++) {
        if(!(p=fork())) {
            for(k=0; k<(NUMBER/CHILD); k++) {
                sem_wait(full);
                sem_wait(mutex);

                /*获得读取位置*/
                lseek(fd, 10*sizeof(int), SEEK_SET);
                read(fd, (char *)&buf_out, sizeof(int));
                /*读取数据*/
                lseek(fd, buf_out*sizeof(int), SEEK_SET);
                read(fd, (char *)&data, sizeof(int));
                /*写入读取位置*/
                buf_out = (buf_out + 1) % BUFSIZE;
                lseek(fd, 10*sizeof(int), SEEK_SET);
                write(fd, (char *)&buf_out, sizeof(int));
                printf("%d: %d\n", getpid(), data);
                sem_post(mutex);
                sem_post(empty);

                fflush(stdout);
            }
            return 0;
        }

        if (p < 0) {
            perror("Failed to fork consumer process!\n");
            return -1;
        }

    }

    wait(NULL);
    sem_unlink("full");
    sem_unlink("empty");
    sem_unlink("mutex");
    close(fd);
    return 0;
}
