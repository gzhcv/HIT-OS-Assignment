#define __LIBRARY__
#include <linux/kernel.h>
#include <unistd.h>
#include <linux/sched.h>
#include <asm/segment.h>
#include <asm/system.h>

#define SHARE_MEM_NUM 5

/* 用一个数组维护key对应的shmid， 数组的索引即shmid，元素值是key */
int key_arr[SHARE_MEM_NUM];
int key_i;
int key_num = 0; /* 现有shmid数量 */

/* 创建的共享内存的物理地址 */
unsigned long page_addr_arr[SHARE_MEM_NUM];
unsigned long page_addr, virtual_addr_tmp;

int sys_shmget(key_t key, size_t size) {
    if (size > 4096) {
        printk("allocte memory size large than 4096B!\n");
        return -1;
    }

    for (key_i = 0; key_i < key_num; key_i++) {
        if (key_arr[key_i] == key)
            return key_i;
    }

    if (key_num == SHARE_MEM_NUM) {
        printk("no shared memory space left!\n");
        return -1;
    }

    page_addr = get_free_page();
    if (page_addr == 0) {
        printk("no memory space left!\n");
    }
    key_arr[key_num] = key;
    page_addr_arr[key_num] = page_addr;
    key_num++;

    return key_num-1;
}

void *sys_shmat(int shmid) {
    if (shmid < 0 || shmid >= key_num) {
        printk("shmid does not exist!\n");
    }
    page_addr = page_addr_arr[shmid];

    /*
    建立虚拟地址与物理地址的映射。
    brk是当前进程虚拟地址空间中未使用的位置。
    此处默认了进程只会使用一个共享内存页，因为虚拟地址一直用的 current->brk+current->ldt[1]。
    */
    /* virtual_addr_tmp = current->brk + get_base(current->ldt[1]); */
    virtual_addr_tmp = current->brk + current->start_code;
    put_page(page_addr, virtual_addr_tmp);
    //write_mem_map(page_addr);

    return (void*)current->brk;
}