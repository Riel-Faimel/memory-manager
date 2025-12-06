# memory-manager
can be used for memory alloc from .bss

64kB storage default and 64B block size 

## 不要问，拿去就是用
可能会有亿点点不安全，但是就是快，安全是留给死人的，快就完了。

## 没啥内部结构
就是纯粹的指针计算，简单包装了一下，所以他很
# 快！！！


# 使用
MemMgr_Blk类，默认64MB总容量，64B块大小，在.hpp文件中定义。内置实例mm，有need(), free()方法：

bool need(void* *data, int size = 1024)

bool free(void* *data, int size = 1024)

没有抛出错误，没有默认释放，调用的时候别忘了。
可以通过调整参数实现不同的分配器，但是需要重新编译。内存池在.bss，不陷入内核。开头有保护区，但是保护不了任何东西。
