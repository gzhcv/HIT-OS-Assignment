该项目记录学习HIT操作系统课程中的一些练习代码。运行实验环境依赖参考:

- https://hoverwinter.gitbooks.io/hit-oslab-manual/content/index.html
- https://github.com/hoverwinter/HIT-OSLab

master分支整合了所有实验的代码，其他分支分别代表一个实验的完整linux代码

## 共享内存实验

### 遇到的问题
一开始为了简化测试代码，生产者消费者分别一个进程，生产者一次性往共享内存写完数据，消费者再去共享内存读。
该代码在ubuntu18.04上能跑通，在linux0.11上会报错，因为生产者进程写完数据后进程退出，
操作系统会释放进程页表记录的物理内存。为啥ubuntu18.04上共享内存似乎没有释放呢？
