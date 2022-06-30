# lab6实验报告

**黄雨石20376156**

[toc]

## 实验思考题

### Thinking6.1

**示例代码中，父进程操作管道的写端，子进程操作管道的读端。如果现在想让父进程作为“读者”，代码应当如何修改？**

**answer:**

```c
father_process
     close(fildes[1]); /* Write end is unused */
     read(fildes[0], buf, 100); /* Get data from pipe */
     printf("father-process read:%s",buf); /* Print the data */
     close(fildes[0]);
     exit(EXIT_SUCCESS);
```

### Thinking6.2

**上面这种不同步修改 pp_ref 而导致的进程竞争问题在 user/fd.c 中的dup 函数中也存在。请结合代码模仿上述情景，分析一下我们的 dup 函数中为什么会出现预想之外的情况？**

**answer:**`dup`函数的功能是将一个文件描述符（例如``fd0``）所对应的内容映射到另一个文件描述符（例如``fd1``）中。这个函数最终会将``fd0``和``pipe``的引用次数都增加$1$，将``fd1``的引用次数变为``fd0``的引用次数。若在复制了文件描述符页面后产生了**时钟中断**，``pipe``的引用次数没来的及增加，可能会导致另一进程调用`pipeisclosed`，发现`pageref(fd[0]) = pageref(pipe)`，误以为读/写端已经关闭。

### Thinking6.3

**阅读上述材料并思考：为什么系统调用一定是原子操作呢？如果你觉得不是所有的系统调用都是原子操作，请给出反例。希望能结合相关代码进行分析。**

**answer:**在进行系统调用时，系统陷入内核，会关闭时钟中断。

```assembly
 .macro CLI
     mfc0 t0, CP0_STATUS 
     li t1, (STATUS_CU0 | 0x1) 
     or t0, t1 
     xor t0, 0x1 
     mtc0 t0, CP0_STATUS 
 .endm
```

### Thinking6.4

**仔细阅读上面这段话，并思考下列问题**

**• 按照上述说法控制 pipeclose 中 fd 和 pipe unmap 的顺序，是否可以解决上述场景的进程竞争问题？给出你的分析过程。**

**• 我们只分析了 close 时的情形，那么对于 dup 中出现的情况又该如何解决？请模仿上述材料写写你的理解。**

**answer:**$1$）可以解决，可以；因为原情况出现的原因是：`a, b` 二值， `a > b` 当先减少 `a` 再减少 `b` 时，就可能会出现 `a == b` 的中间态；改变顺序后， `b` 先减少，此时 `a > b > b*` ，不会出现 `a == b` 这种情况$2$）`dup`也会出现同样的问题，先对`pipe`进行``map``，再对`fd`进行``map``即可。

### Thinking 6.5

`bss` 在 `ELF` 中并不占空间，但 `ELF` 加载进内存后， `bss` 段的数据占据了空间，并且初始值都是 `0`。

- 请回答你设计的函数是如何实现上面这点的？

**answer:**`Load` 二进制文件时，根据 `bss` 段数据的 `memsz` 属性分配对应的内存空间并清零

### Thinking 6.6

**为什么我们的 \*.b 的 text 段偏移值都是一样的，为固定值？**

**answer:**在**user/user.lds**文件中约定了``text``段地址为``0x00400000``

### Thinking 6.7

**在 shell 中执行的命令分为内置命令和外部命令。在执行内置命令时 shell 不需要 fork 一个子 shell，如 Linux 系统中的 cd 指令。在执行外部命令时 shell 需要 fork 一个子 shell，然后子 shell 去执行这条命令。据此判断，在 MOS 中我们用到的 shell 命令是内置命令还是外部命令？请思考为什么 Linux 的 cd 指令是内部指令而不是外部指令？**

**answer:**我们用到的``shell``命令是外部命令，因为我们的``user``文件夹中有`cat.c` `ls.c`文件，Linux下的``cd``指令没有对应的文件，使用时也不需要单独的创建一个子进程。``cd`` 所做的是改变`` shell`` 的 ``PWD``。 因此倘若 ``cd`` 是一个外部命令，那么它改变的将会是子 ``shell`` 的 ``PWD``，也不会向父 ``shell`` 返回任何东西。所以，当前 ``shell`` 的 ``PWD`` 就不会做任何改变。**所有能对当前 ``shell``的环境作出改变的命令都必须是内部命令。** 因此如果我们将 ``cd`` 做成外部命令，就无法像原来一样改变当前目录了。

### Thinking 6.8

**在哪步，0 和 1 被 “安排” 为标准输入和标准输出？请分析代码执行流程，给出答案。**

**answer:**在`user/init`函数中可以看到如下步骤，它将0映射在1上，相当于就是把控制台的输入输出缓冲区当做管道。

```lisp
 if ((r = dup(0, 1)) < 0)
         user_panic("dup: %d", r);
```

### Thinking 6.9

**在你的 shell 中输入指令 ls.b | cat.b > motd。**

**• 请问你可以在你的 shell 中观察到几次 spawn？分别对应哪个进程？**

**• 请问你可以在你的 shell 中观察到几次进程销毁？分别对应哪个进程？**

**answer:**$1$）两次，分别对应`[00001c03] SPAWN: ls.b`、`[00002404] SPAWN: cat.b`$2$）五次，分别对应``[00000400] destroying 00000400 icode.b``、`[00003406] destroying 00003406 sh.b`、`[00002c05] destroying 00002c05 init.b`、`[00002404] destroying 00002404 cat.b`、`[00001c03] destroying 00001c03 ls.b `

## 实验难点图示

* ``spawn``过程![spawn](https://github.com/VOIDMalkuth/BUAA_OS_2019_Code/raw/master/%E5%AE%9E%E9%AA%8C%E6%8A%A5%E5%91%8A/Lab6%20Report/spawn.png)

## 体会与感想

本``lab``难度适中，但是许多知识都涉及到之前的``lab``因此需要重新复习之前的``lab``都相关知识。耗时大概在$6$个小时左右，主要在理解``sh.c``中相关函数上花费较久的时间，但是这个``lab``做完以后非常有成就感，特别是看到自己的操作系统终于可以运行一些简单的``shell``指令时，感觉到这个学期的努力都是值得的
