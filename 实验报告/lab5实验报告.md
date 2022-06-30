# lab5实验报告

**黄雨石20376156**

[toc]

## 实验思考题

### Thinking5.1

**查阅资料，了解 Linux/Unix 的 /proc 文件系统是什么？有什么作用？ Windows 操作系统又是如何实现这些功能的？proc 文件系统这样的设计有什么好处和可以改进的地方？**

**answer:**/proc文件系统是一个**虚拟文件系统**，只存在于内存中，而不占用外存空间。它提供了新的一种在用户空间和内核空间中进行**通信**的模式。用户和应用程序可以通过proc得到系统的信息，并可以改变内核的某些参数，这个过程是**动态**的，随着内核参数的改变而改变。用户可以获取内核信息。

windows:**通过使用`WMI`来实现相似功能的**——`WMI`是可伸缩的系统管理结构，该规范采用⼀个统一、基于标准且可扩展的面向对象接口。它提供与系统管理员信息和基础`WMI API`交互的标准方法，主要由系统管理应用程序开发⼈员和系统管理员⽤来访问和操作系统管理信息；它可用来生成组织和管理系统信息的工具，使系统管理⼈员能够更密切的监视系统活动。

好处：通过对文件系统的操作进行用户空间与内核空间的通信，更加方便快捷，不用像**系统调用**一样再**陷入内核态**。

改进地方：但访问的容易必将带来安全性的隐患，可能会对一些危险操作不设防吧？需要进一步判断改进。

### Thinking5.2

**如果我们通过 kseg0 读写设备，我们对于设备的写入会缓存到 Cache 中。通过 kseg0 访问设备是一种错误的行为，在实际编写代码的时候这么做会引发不可预知的问题。请你思考：这么做这会引起什么问题？对于不同种类的设备（如我们提到的串口设备和 IDE 磁盘）的操作会有差异吗？可以从缓存的性质和缓存刷新的策略来考虑。**

**anwser:** 当外部设备自身更新数据时，如果此时CPU写入外设的数据还只在缓存中，**则缓存的那部分数据就只能在外设自身更新后再写入外设**（只有缓存块将要被新进入的数据取代时，缓存数据才会被写入内存），这样就会发生错误的行为。

串口设备读写频繁，而IDE磁盘读写频率相对较小，因此串口设备发生错误的概率要远大于IDE磁盘。

### Thinking5.3

**比较 MOS 操作系统的文件控制块和 Unix/Linux 操作系统的 inode 及相关概念，试述二者的不同之处。**

**answer:**

```cpp
 struct inode_operations {
     int (*create) (struct inode *,struct dentry *,int);
     struct dentry * (*lookup) (struct inode *,struct dentry *);
     int (*link) (struct dentry *,struct inode *,struct dentry *);
     int (*unlink) (struct inode *,struct dentry *);
     int (*symlink) (struct inode *,struct dentry *,const char *);
     ...
 };
 
 struct file_operations {
     struct module *owner;
     loff_t (*llseek) (struct file *, loff_t, int);
     ssize_t (*read) (struct file *, char *, size_t, loff_t *);
     ssize_t (*write) (struct file *, const char *, size_t, loff_t *);
     ...
 };
 
 struct inode {
     ...
     unsigned long       i_ino;
     atomic_t            i_count;
     kdev_t              i_dev;
     umode_t             i_mode;
     nlink_t             i_nlink;
     uid_t               i_uid;
     gid_t               i_gid;
     kdev_t              i_rdev;
     loff_t              i_size;
     time_t              i_atime;
     time_t              i_mtime;
     time_t              i_ctime;
     ...
     struct inode_operations *i_op;
     struct file_operations  *i_fop;
     struct super_block      *i_sb;
     ...
 };
```

Linux系统下FCB储存了文件的所有信息，但索引结点下的FCB 进行了改进，除了**文件名**之外的文件描述信息都放到**索引节点**里，这样一来该结构体大小就可以匹配磁盘块的大小，大大提升文件检索速度。如此一来inode模块则封装了很多东西，我们应该重点关注 `i_op` 和 `i_fop` 这两个成员。``i_op`` 成员定义对目录相关的操作方法列表，譬如`` mkdir()``系统调用会触发 ``inode->i_op->mkdir() ``方法，而 ``link() ``系统调用会触发 ``inode->i_op->link()`` 方法。而`` i_fop`` 成员则定义了对打开文件后对文件的操作方法列表，譬如`` read()`` 系统调用会触发`` inode->i_fop->read() ``方法，而`` write() ``系统调用会触发`` inode->i_fop->write() ``方法。

```cpp
 struct File {
     u_char f_name[MAXNAMELEN];  // filename
     u_int f_size;           // file size in bytes
     u_int f_type;           // file type
     u_int f_direct[NDIRECT];
     u_int f_indirect;
 
     struct File *f_dir;     
     // the pointer to the dir where this file is in, valid only in memory.
     u_char f_pad[BY2FILE - MAXNAMELEN - 4 - 4 - NDIRECT * 4 - 4 - 4];
 };
 // file descriptor + file
 struct Filefd {
     struct Fd f_fd;
     u_int f_fileid;
     struct File f_file;
 };
 // file descriptor
 struct Fd {
     u_int fd_dev_id;
     u_int fd_offset;
     u_int fd_omode;
 };
 struct Dev {
     int dev_id;
     char *dev_name;
     int (*dev_read)(struct Fd *, void *, u_int, u_int);
     int (*dev_write)(struct Fd *, const void *, u_int, u_int);
     int (*dev_close)(struct Fd *);
     int (*dev_stat)(struct Fd *, struct Stat *);
     int (*dev_seek)(struct Fd *, u_int);
 };
```

我们实验所编写的MOS文件控制块不是叫的FCB，而是File，对文件进行操作的函数却全都实现在了Dev结构体的接口里。此外我们对文件的操作是依靠进程间通信来完成的，而Linux是直接系统调用完成的。总的来说我们的MOS文件系统性能变低了，但用户空间实现使得可靠性变高了。

### Thinking5.4

 **查找代码中的相关定义，试回答一个磁盘块中最多能存储多少个文件 控制块？一个目录下最多能有多少个文件？我们的文件系统支持的单个文件最大为 多大？**

**answer:**通过查看文件控制块的定义，我们可以发现，每个文件控制块都被数组`f_pad`强制对齐为$256B$。

```c
struct File {
	u_char f_name[MAXNAMELEN];	
	u_int f_size;			
	u_int f_type;			
	u_int f_direct[NDIRECT];
	u_int f_indirect;
	struct File *f_dir;		
    //BY2FILE = 256
	u_char f_pad[BY2FILE - MAXNAMELEN - 4 - 4 - NDIRECT * 4 - 4 - 4];
};
```

一个磁盘块的容量为$4KB$，因此最多可以容纳$16$个文件控制块。
一个目录文件最多可以使用$1024$个磁盘块存储数据，因此一个目录下最多$1024\times16 = 16384$个文件。
一个文件最多可以使用$1024$个磁盘块存储数据，因此一个文件最大容量为$1024\times 4KB = 4MB$。

### Thinking 5.5

**请思考，在满足磁盘块缓存的设计的前提下，我们实验使用的内核支持的最大磁盘大小是多少？**

**answer:** 块缓存所在的地址空间为`[0x10000000, 0x50000000)`,因此我们的内核能够支持的磁盘大小为`0x40000000`,也就是$1GB$。

### Thinking 5.6

**如果将 DISKMAX 改成 0xC0000000, 超过用户空间，我们的文件系统还能正常工作吗？为什么？**
**answer:** 不可以正常工作。如果`DISHMAX`值为`0xC0000000`，则有一部分块缓存会在内核空间中。而我们的文件系统属于文件用户态进程，无法访问内核空间的数据（访问内核空间数据时会出现`TOO LOW`报错）。

### Thinking 5.7

**在 lab5 中，fs/fs.h、include/fs.h 等文件中出现了许多宏定义，试列举你认为较为重要的宏定义，并进行解释，写出其主要应用之处。**

**answer:**

```cpp
 #define BY2SECT     512 /* Bytes per disk sector */
 #define SECT2BLK    (BY2BLK/BY2SECT)    /* sectors to a block */
 /* Disk block n, when in memory, is mapped into the file system
  * server's address space at DISKMAP+(n*BY2BLK). */
 #define DISKMAP     0x10000000
 /* Maximum disk size we can handle (1GB) */
 #define DISKMAX     0x40000000
 #define BY2BLK      BY2PG
 // Maximum size of a filename (a single path component), including null
 #define MAXNAMELEN  128
 
 struct File {
     u_char f_name[MAXNAMELEN];  // filename
     u_int f_size;           // file size in bytes
     u_int f_type;           // file type
     u_int f_direct[NDIRECT];
     u_int f_indirect;
 
     struct File *f_dir;     // the pointer to the dir where this file is in, valid only in memory.
     u_char f_pad[BY2FILE - MAXNAMELEN - 4 - 4 - NDIRECT * 4 - 4 - 4];
 };
 // File types
 #define FTYPE_REG       0   // Regular file
 #define FTYPE_DIR       1   // Directory
```

从上到下依次是1个扇区大小512字节，1个磁盘块是8个扇区，缓冲区地址范围为0x10000000--0x3ffffffff，一个磁盘块大小等于一个页面大小（4KB），文件名最长为128个char，File结构体用于索引，文件类型为文件或文件夹。

前面的宏主要用于内存分配，后面的宏主要用于用户操作。

### Thinking 5.8

**阅读 user/file.c，你会发现很多函数中都会将一个 struct Fd型的 指针转换为 struct Filefd 型的指针，请解释为什么这样的转换可行。**

**answer:** 首先，每个文件描述符fd都独占一页的，远大于``struct Fd``和`struct Filefd`的大小，因此将`struct Fd*`强制转换为`struct Filefd*`不会访问其他不相关的数据。
其次，每个`strcut Filefd`的开头都包含一个`struct File`，将`struct Fd*`强制转换为`struct Filefd*`后，同样可以通过`p->f_fd`来访问原先`struct Fd`中数据。这和面向对象中基类和派生类的关系十分相似，如果将`struct Fd`想象成基类，`struct File`想象成派生类，将基类的指针"向下转型"到派生类的指针是没有任何问题的。

### Thinking 5.9

**在lab4 的实验中我们实现了极为重要的fork 函数。那么fork 前后的父子进程是否会共享文件描述符和定位指针呢？请在完成练习5.8和5.9的基础上编写一个程序进行验证。**

**answer:** 一个进程所有的文件描述符都存储在`[FDTABLE, FILEBASE)`这一地址空间中。在`fork`函数执行时，会将这父进程页表中映射一部分地址的页表项拷贝到子进程的页表中，因此`fork`前后的父子进程会共享文件描述符和定位指针。

### Thinking 5.10

**请解释Fd, Filefd, Open 结构体及其各个域的作用。比如各个结构体会在哪些过程中被使用，是否对应磁盘上的物理实体还是单纯的内存数据等。说明形式自定，要求简洁明了，可大致勾勒出文件系统数据结构与物理实体的对应关系与设计框架。**

**answer:**

- **Fd结构体定义及各个域的作用如下所示**

  ```c
  struct Fd {
  	u_int fd_dev_id;    //文件对应的设备id
  	u_int fd_offset;    //文件指针所指向的位置
  	u_int fd_omode;     //文件打开模式
  };
  ```

  该结构体主要用于记录已打开文件的状态，便于用户直接使用文件描述符对文件进行操作、申请服务等等。由于文件描述符主要是为用户所使用，因此它对应的是磁盘映射到内存中的数据。

- **Filefd的结构体定义及各个域的作用如下所示**

  ```c
  struct Filefd {
  	struct Fd f_fd;     //文件描述符结构体
  	u_int f_fileid;     //文件id，表示该文件在opentab中的位置
  	struct File f_file; //文件控制块
  };
  ```

  文件描述符中存储的数据毕竟是有限的，有的时候我们需要将`Fd*`强制转换为`Filefd*`从而获取到文件控制块，从而获得更多文件信息，比如文件大小等等。

- **Open结构体定义及各个域的作用如下所示**

  ```c
  struct Open {
  	struct File *o_file;	// 文件控制块指针
  	u_int o_fileid;		    // 文件id
  	int o_mode;		        // 文件打开方式
  	struct Filefd *o_ff;	// Filefd结构体
  };
  ```

  这个结构体主要被文件系统服务进程使用，用于记录所有进程中被打开的文件。每个被打开的文件都对应一个`Open`结构体，所有的结构体依次排列在数组`opentab[MAXOPEN]`，每次我们只需要通过`file_id`来查找即可。

### Thinking 5.11

**UML时序图中有多种不同形式的箭头，请结合UML 时序图的规范，解释这些不同箭头的差别，并思考我们的操作系统是如何实现对应类型的进程间通信的。**

**answer:** UML时序图中有以下几种箭头——

|        消息类型        |                           表示                           |
| :--------------------: | :------------------------------------------------------: |
|        同步消息        |                用黑三角箭头搭配黑实线表示                |
|        异步消息        |             用两条小线的开箭头和黑色实线表示             |
|        返回消息        |               用黑三角箭头搭配黑色虚线表示               |
|        创建消息        | 用开三角箭头搭配黑实线表示，其下面特别注明 `<<create>>`  |
|        摧毁消息        | 用黑三角箭头搭配黑实线表示，其下面特别注明 `<<destroy>>` |
| Lost and Found Message |      用一个黑色实心的点和黑色实心三角箭头黑实线表示      |

我们的操作系统是通过IPC来实现进程间通信的，这种方式传递的信息本质上是一种同步消息。具体流程是：发送方先调用`ipc_send`函数，该函数通过一个死循环来不断向接收方发送信息。当接收方成功接收到消息时，`ipc_send`函数跳出循环并结束，这时发送方再调用`ipc_recv`函数主动放弃CPU，等待接收返回信息。

整个流程在函数`fsipc`里得到体现

```c
static int fsipc(u_int type, void *fsreq, u_int dstva, u_int *perm)
{
	u_int whom;
	ipc_send(envs[1].env_id, type, (u_int)fsreq, PTE_V | PTE_R);
	return ipc_recv(&whom, dstva, perm);
}
```

### Thinking 5.12

**阅读serv.c/serve函数的代码，我们注意到函数中包含了一个死循环`for (;;) {...}`，为什么这段代码不会导致整个内核进入panic 状态？**

**answer:** 这个死循环并不会导致CPU的轮询，原因是每次循环会调用`ipc_recv`函数。在这个函数被调用后，文件系统服务进程就会主动让出CPU（进程状态被标记为`NOT_RANNABLE`），直到用户进程向该进程申请服务（调用`ipc_send`）时才会被唤醒，进而为用户进程提供服务。因此并不会轮询导致内核进入`panic`状态。

## 实验难点图示

我认为本次实验主要有以下难点：

- 目录文件控制块在磁盘上的存储
- 文件系统中服务进程的地址空间布局和用户进程的地址空间布局
- 服务进程与用户进程的交互的函数调用关系

由于考期将近，以及这一个``lab``难度太大太花时间，其实还有好多难点就不一一画图列出了

* **目录文件控制块在磁盘上的存储：**

  ![img](https://hyggge.github.io/2022/06/04/OS-Lab5%E5%AE%9E%E9%AA%8C%E6%8A%A5%E5%91%8A/file_data.drawio.svg)

* **文件系统中服务进程的地址空间布局和用户进程的地址空间布局**

  <img src="https://thysrael.github.io/posts/c8a10fd9/image-20220607162418190.png" alt="image-20220607162418190" style="zoom:50%;" />

  <img src="https://thysrael.github.io/posts/c8a10fd9/image-20220607162515076.png" alt="image-20220607162515076" style="zoom:50%;" />

* **服务进程与用户进程的交互的函数调用关系**

  <img src="https://img2022.cnblogs.com/blog/2807082/202206/2807082-20220607101941782-245080127.png" alt="img" style="zoom:80%;" />

## 体会与感想

感觉``lab5``是难度最大的一个``lab``但同时也是难度最畸形的一个``lab``，前半部分可以说简单到离谱，但是后半部分不仅仅是指导书不太清楚，函数关系也非常复杂。填的只是其中九牛一毛的代码，不花费大量时间根本无法看清用户进程和服务进程交互的完整流程，同时建议这一个``lab``的指导书真的要写详细一点（求求了，特别是``lab5``已近考期，真的非常心累，建议给出主要有用的每个函数的调用关系，像``lab2``一样，并且这个``lab``还比``lab2``难辣么多QAQ），光看指导书大脑中完全无法形成文件系统的结构以及大体运行框架，更不要说细节了），这个``lab``大概花费了``20``个小时完成，非常肝。
