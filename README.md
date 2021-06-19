该项目记录学习HIT操作系统课程中的一些练习代码。运行实验环境依赖参考:

- https://hoverwinter.gitbooks.io/hit-oslab-manual/content/index.html
- https://github.com/hoverwinter/HIT-OSLab

master分支整合了所有实验的代码，其他分支分别代表一个实验的完整linux代码

## 信号量实验

### 编写生产者消费者模型代码

- 由于有多个消费者进程，读取文件数据的位置需要同步，因此在文件缓存区中维护一个数据表示当前应读的位置。
- 所有打开的文件都有一个当前**文件偏移量cfo**(current file offset)。表示当前读写的位置。lseek函数可以设置当前读写的位置。

### 本地数据传到bochs模拟器

1. 挂载hdc ```sudo ./mount-hdc```
2. 拷贝文件到hdc ```cp xxx hdc/```
3. 卸载hdc ```sudo unmount hdc```
3. 启动linux0.11 ```./run```
4. 在bochs中切到根目录下便能找到拷贝的文件 ```cd /```

### bochs模拟器数据传到本地

- 注意1：不要在0.11内核运行的时候mount镜像文件，否则可能会损坏文件系统。同理，也不要在已经mount的时候运行0.11内核。

- 注意2：在关闭Bochs之前，需要先在0.11的命令行运行“sync”，确保所有缓存数据都存盘后，再关闭Bochs。

### 遇到的问题
- 在bochs中编译pc.c时遇到``` __NR__sem_open undeclared (first use this function) ```,
由于这个是定义在```unistd.h```中(系统头文件位置```/usr/include```)。所以需要手动将该头文件从本地拷贝到bochs。

- 测试结果不稳定，有时候屏幕打印正常，有时候打印全0。
