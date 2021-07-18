#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>

#include <linux/sched.h>
#include <linux/kernel.h>
#include <asm/segment.h>

char proc_buf[4096] ={'\0'};

int sprintf(char *buf, const char *fmt, ...)
{
    va_list args; int i;
    va_start(args, fmt);
    i=vsprintf(buf, fmt, args);
    va_end(args);
    return i;
}

void read_psinfo(struct m_inode * inode) {
    struct task_struct **p;
    if (inode->i_size == 0) {
        inode->i_size += sprintf((proc_buf+inode->i_size), "%s %s %s %s %s\n",
          "pid", "state", "father", "counter", "start_time");
        for (p = &FIRST_TASK; p <= &LAST_TASK; p++) {
            if (*p) {
                inode->i_size += sprintf((proc_buf+inode->i_size), "%d %d %d %d %d\n",
                  (*p)->pid, (*p)->state, (*p)->father, (*p)->counter, (*p)->start_time);
            }
        }
    }
}

void read_hdinfo(struct m_inode * inode) {
    if (inode->i_size == 0) {
        int total_blocks, total_inodes;
        struct super_block * sb;
        sb=get_super(inode->i_dev);
        struct buffer_head * bh;
        total_blocks = sb->s_nzones;
        total_inodes = sb->s_ninodes;
//        for(i=0; i < sb->s_zmap_blocks; i++)
//        {
//            bh = sb->s_zmap[i];
//            p=(char *)bh->b_data;
//        }
        inode->i_size += sprintf((proc_buf+inode->i_size), "%s %s\n",
          "total_blocks", "total_inodes");
        inode->i_size += sprintf((proc_buf+inode->i_size), "%d %d\n",
          total_blocks, total_inodes);
    }
}

int proc_read(struct m_inode * inode, int * f_pos, char * buf, int count) {
    // psinfo
    int left;

    if (inode->i_zone[0] == 0)
        read_psinfo(inode);
    if (inode->i_zone[0] == 1)
        read_hdinfo(inode);

    if (count+(*f_pos) > inode->i_size)
        count = inode->i_size - (*f_pos);
    left = count;
    while(left--)
        put_fs_byte(proc_buf[(*f_pos)++], buf++);

    if (count == 0)
        inode->i_size = 0;
    // hdinfo

    return count;
}