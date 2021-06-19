#define __LIBRARY__
#include <linux/kernel.h>
#include <unistd.h>
//#include <string.h>
#include <linux/sched.h>
#include <asm/segment.h>
#include <asm/system.h>

sem_t sem_arr[SEM_LEN];
int sem_num = 0; /* 现有信号量的数量 */
int sem_i;

void init_queue(task_queue* q) {
    q->front = q->rear = 0;
}

int is_empty(task_queue* q) {
    return q->front == q->rear ? 1:0;
}

int is_full(task_queue* q) {
    return (q->rear+1)%SEM_LEN == q->front? 1:0;
}

/* 获得队列头第一个任务 */
struct task_struct* get_task(task_queue* q) {
    if (is_empty(q)) {
        printk("Queue is empty\n");
        return NULL;
    }
    struct task_struct *tmp = q->tasks[q->front];
    q->front = (q->front+1)%SEM_LEN;
    return tmp;
}

/* 任务插入队列尾部 */

int insert_task(struct task_struct *p, task_queue *q) {
    if (is_full(q)) {
        printk("Queue is full\n");
        return -1;
    }
    q->tasks[q->rear] = p;
    q->rear = (q->rear+1)%SEM_LEN;
    return 1;
}

sem_t* sys_sem_open(const char *name, unsigned int value) {
    int i;
    char c;
    char tmp[STR_LEN];
    for (i = 0; i < STR_LEN; i++) {
        c = get_fs_byte(name[i]);
        tmp[i] = c;
        if (c == '\0')
            break;
    }
    /* 从现有信号量里搜索 */
    if (sem_num > 0) {
        for (sem_i = 0; sem_i < sem_num; sem_i++) {
            if (strcmp(tmp, sem_arr[sem_i].name) == 0) {
                printk("find exist sem!\n");
                return (&sem_arr[sem_i]);
            }
        }
    }

    /* 超出能创建的信号量上限 */
    if (sem_num == SEM_LEN) {
        printk("Exceed the max number of semaphores\n");
        return NULL;
    }

    /* 新建一个信号量 */
    sem_arr[sem_num].value = value;
    strcpy(sem_arr[sem_num].name, tmp);
    init_queue(&(sem_arr[sem_num].wait_queue));
    sem_num++;

    return &sem_arr[sem_num-1];
}

int sys_sem_wait(sem_t *sem) {
    cli();
    sem->value--;

    if (sem->value < 0) {
        current->state = TASK_UNINTERRUPTIBLE;
        insert_task(current, &(sem->wait_queue));
        schedule();
    }

    sti();
    return 0;
}

int sys_sem_post(sem_t *sem) {
    cli();
    struct task_struct* p;
    sem->value++;

    if (sem->value <= 0) {
        p = get_task(&(sem->wait_queue));
        if (p != NULL) {
            (*p).state = TASK_RUNNING;
        }
    }

    sti();
    return 0;
}

int sys_sem_unlink(const char *name) {
    int i;
    char c;
    char tmp[STR_LEN];
    for (i = 0; i < STR_LEN; i++) {
        c = get_fs_byte(name[i]);
        tmp[i] = c;
        if (c == '\0')
            break;
    }
    for (sem_i = 0; sem_i < SEM_LEN; sem_i++) {
        if (strcmp(sem_arr[sem_i].name, tmp) == 0) {
            for (; sem_i < (SEM_LEN-1); sem_i++)
                sem_arr[sem_i] = sem_arr[sem_i+1];
            return 0;
        }
    }

    return -1;
}