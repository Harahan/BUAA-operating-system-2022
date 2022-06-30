# lab1实验报告

**黄雨石20376156**

[toc]

## 实验思考题

### Thinking1.1

* 请查阅并给出前述objdump 中使用的参数的含义。使用其它体系结构的编译器（如课程平台的MIPS交叉编译器）重复上述各步编译过程，观察并在实验报告中提交相应结果。

  **answer:使用``man objdump ``，可以发现，对于``objdump -DS``，-D表示反汇编所有节内容（disassemble the contents of all sections），-S参数表示显示与反汇编混合状态下的源代码（Display source code intermixed with disassembly, if possible.）**

  **使用命令如下：**

  ```
  /OSLAB/compiler/usr/bin/mips_4KC-gcc -c helloworld.c
   /OSLAB/compiler/usr/bin/mips_4KC-ld helloworld.o -o helloworld
   /OSLAB/compiler/usr/bin/mips_4KC-objdump -DS helloworld.o > o_out.txt
   /OSLAB/compiler/usr/bin/mips_4KC-objdump -DS helloworld > out.txt
  ```

  **反编译的.o文件：**

  ```
  helloworld.o:     file format elf32-tradbigmips
  
  Disassembly of section .text:
  
  00000000 <main>:
     0:   27bdffe8        addiu   sp,sp,-24
     4:   afbe0010        sw      s8,16(sp)
     8:   03a0f021        move    s8,sp
     c:   afc00008        sw      zero,8(s8)
    10:   8fc20008        lw      v0,8(s8)
    14:   24420003        addiu   v0,v0,3
    18:   afc20008        sw      v0,8(s8)
    1c:   00001021        move    v0,zero
    20:   03c0e821        move    sp,s8
    24:   8fbe0010        lw      s8,16(sp)
    28:   27bd0018        addiu   sp,sp,24
    2c:   03e00008        jr      ra
    30:   00000000        nop
          ...
  Disassembly of section .reginfo:
  
  00000000 <.reginfo>:
     0:   e0000004        sc      zero,4(zero)
          ...
  Disassembly of section .pdr:
  
  00000000 <.pdr>:
     0:   00000000        nop
     4:   40000000        mfc0    zero,c0_index
     8:   fffffff8        sdc3    $31,-8(ra)
          ...
    14:   00000018        mult    zero,zero
    18:   0000001e        0x1e
    1c:   0000001f        0x1f
  Disassembly of section .comment:
  
  00000000 <.comment>:
     0:   00474343        0x474343
     4:   3a202847        xori    zero,s1,0x2847
     8:   4e552920        c3      0x552920
     c:   342e302e        ori     t6,at,0x302e
    10:   30202844        andi    zero,at,0x2844
    14:   454e5820        0x454e5820
    18:   454c444b        0x454c444b
    1c:   20342e31        addi    s4,at,11825
    20:   20342e30        addi    s4,at,11824
    24:   2e302900        sltiu   s0,s1,10496
  ```

  **反编译可执行文件：**

  ```
  helloworld:     file format elf32-tradbigmips
  
  Disassembly of section .reginfo:
  
  00400094 <.reginfo>:
    400094:       e0000004        sc      zero,4(zero)
          ...
    4000a8:       10007ff0        b       42006c <main+0x1ffbc>
  Disassembly of section .text:
  
  004000b0 <main>:
    4000b0:       27bdffe8        addiu   sp,sp,-24
    4000b4:       afbe0010        sw      s8,16(sp)
    4000b8:       03a0f021        move    s8,sp
    4000bc:       afc00008        sw      zero,8(s8)
    4000c0:       8fc20008        lw      v0,8(s8)
    4000c4:       24420003        addiu   v0,v0,3
    4000c8:       afc20008        sw      v0,8(s8)
    4000cc:       00001021        move    v0,zero
    4000d0:       03c0e821        move    sp,s8
    4000d4:       8fbe0010        lw      s8,16(sp)
    4000d8:       27bd0018        addiu   sp,sp,24
    4000dc:       03e00008        jr      ra
    4000e0:       00000000        nop
          ...
  Disassembly of section .comment:
  
  00000000 <.comment>:
     0:   00474343        0x474343
     4:   3a202847        xori    zero,s1,0x2847
     8:   4e552920        c3      0x552920
     c:   342e302e        ori     t6,at,0x302e
    10:   30202844        andi    zero,at,0x2844
    14:   454e5820        0x454e5820
    18:   454c444b        0x454c444b
    1c:   20342e31        addi    s4,at,11825
    20:   20342e30        addi    s4,at,11824
    24:   2e302900        sltiu   s0,s1,10496
  Disassembly of section .pdr:
  
  00000000 <.pdr>:
     0:   004000b0        tge     v0,zero,0x2
     4:   40000000        mfc0    zero,c0_index
     8:   fffffff8        sdc3    $31,-8(ra)
          ...
    14:   00000018        mult    zero,zero
    18:   0000001e        0x1e
    1c:   0000001f        0x1f
  ```

### Thinking1.2

* **也许你会发现我们的readelf程序是不能解析之前生成的内核文件(内核文件是可执行文件)的，而我们之后将要介绍的工具readelf则可以解析，这是为什么呢？(提示：尝试使用readelf -h，观察不同)**

  **answer:使用``readelf -h testELF``解析testELF文件:**

  ```
   ELF Header:
     Magic:   7f 45 4c 46 01 01 01 00 00 00 00 00 00 00 00 00 
     Class:                             ELF32
     Data:                              2's complement, little endian
     Version:                           1 (current)
     OS/ABI:                            UNIX - System V
     ABI Version:                       0
     Type:                              EXEC (Executable file)
     Machine:                           Intel 80386
     Version:                           0x1
     Entry point address:               0x8048490
     Start of program headers:          52 (bytes into file)
     Start of section headers:          4440 (bytes into file)
     Flags:                             0x0
     Size of this header:               52 (bytes)
     Size of program headers:           32 (bytes)
     Number of program headers:         9
     Size of section headers:           40 (bytes)
     Number of section headers:         30
     Section header string table index: 27
  ```

  **使用``readelf -h vmlinux``解析vmlinux文件:**

  ```
   ELF Header:
     Magic:   7f 45 4c 46 01 02 01 00 00 00 00 00 00 00 00 00 
     Class:                             ELF32
     Data:                              2's complement, big endian
     Version:                           1 (current)
     OS/ABI:                            UNIX - System V
     ABI Version:                       0
     Type:                              EXEC (Executable file)
     Machine:                           MIPS R3000
     Version:                           0x1
     Entry point address:               0x80010000
     Start of program headers:          52 (bytes into file)
     Start of section headers:          37164 (bytes into file)
     Flags:                             0x1001, noreorder, o32, mips1
     Size of this header:               52 (bytes)
     Size of program headers:           32 (bytes)
     Number of program headers:         2
     Size of section headers:           40 (bytes)
     Number of section headers:         14
     Section header string table index: 11
  ```

  **在Data段的信息中可以看到，我们生成的readelf文件适用于解析小端存储的，而我们的内核vmlinux是大端存储的文件，因此无法解析内核。**

### Thinking1.3

* 在理论课上我们了解到，MIPS 体系结构上电时，启动入口地址为``0xBFC00000``（其实启动入口地址是根据具体型号而定的，由硬件逻辑确定，也有可能不是这个地址，但一定是一个确定的地址），但实验操作系统的内核入口并没有放在上电启动地址，而是按照内存布局图放置。思考为什么这样放置内核还能保证内核入口被正确跳转到？**（提示：思考实验中启动过程的两阶段分别由谁执行。）**

  **answer:实验操作系统使用GXemul仿真器，支持直接加载ELF格式的内核。BootLoader中Stage1,Stage2的全部功能已经由其提供，而事实上此时已经跳到了内核入口，然后跳到内核后第一步就是实现在boot/start.o中的代码初始化硬件设备，设置堆栈，跳转到main函数入口。**

### Thinking1.4

* 与内核相比，普通进程的``sg_size ``和``bin_size`` 的区别在于它的开始加载位置并非页对齐，同时``bin_size``的结束位置（``va+i``，其中``i``为计算出的该段在ELF文件中的大小）也并非页对齐，最终整个段加载完毕的``sg_size`` 末尾的位置也并非页对齐。请思考，为了保证页面不冲突（不重复为同一地址申请多个页，以及页上数据尽可能减少冲突），这样一个程序段应该怎样加载内存空间中。

  彻底并透彻地理解上图能帮助大家在后续实验中节约很多时间

  ```
              va(加载起始地址)                                           va+i
  |                                            |                          |
  |_ _ _|___|___BY2PG___|___BY2PG___|___BY2PG__|____|____|___BY2PG___|___ |_ _ _|
  offset|                                      |                    |
        |<---            .text & .data           --->|<---    .bss    --->|
        |<---              bin_size              --->|                    |
        |<---                           sg_size                       --->|
  ```

  **answer:在加载程序时，避免发生冲突页面现象。首先，不同程序段的占用空间不能够有重合，然后，尽量避免一个页面同时被多个程序段所占用。即若前面的程序段末地址所占用的页面地址为$v_i$，则后续的程序段首地址应从下一页面$v_{i+1}$开始占用。**

### Thinking1.5

* 内核入口在什么地方？``main`` 函数在什么地方？我们是怎么让内核进入到想要的 ``main`` 函数的呢？又是怎么进行跨文件调用函数的呢？

  **answer:内核入口是``start.S``中的``_start``函数地址在``0x80010000``，``main``函数在``0x80010050``处。我们在start.S文件中编写mips汇编代码，设置堆栈后直接跳到main函数中。在跨文件调用函数时，每个函数会有一个固定的地址，调用过程为将需要存储的值进行进栈等保护，再用jal跳转到相应函数的地址。**

### Thinking1.6 

* 查阅《See MIPS Run Linux》一书相关章节，解释boot/start.S 中下面几行对CP0 协处理器寄存器进行读写的意义。具体而言，它们分别读/写了哪些寄存器的哪些特定位，从而达到什么目的？

  ```
  /* Disable interrupts */
  mtc0 zero, CP0_STATUS
  ......
  /* disable kernel mode cache */
  mfc0 t0, CP0_CONFIG
  and t0, ~0x7
  ori t0, 0x2
  mtc0 t0, CP0_CONFIG
  ```

  **answer:将宏定义进行转换后如下**

  ```assembly
  /* Disable interrupts */
  mtc0 $0, $12 # 将sr寄存器清零
  ......
  /* disable kernel mode cache */
  mfc0 $t0, $16
  and $t0, ~0x7 
  ori $t0, 0x2
  mtc0 $t0, $16 #将CP0_CONFIG寄存器的0号位和2号位置0，将1号位置1
  ```

  **目的：1. 设置SR寄存器来使CPU进入工作状态，而硬件一般是复位后使许多寄存器的位为未定义行为；2. CONFIG寄存器的后三位为可写为，用来决定固定的kseg0区是否经过高速缓存和其确切行为如何**



## 实验难点图示

### ELF理解

关于ELF格式及其理解：

![image-20220320152412440](C:\Users\hys\AppData\Roaming\Typora\typora-user-images\image-20220320152412440.png)

可以看到左边的每一个不同颜色的块就是一个section，右边的``.text``...就是每一个segment

![img](https://os.buaa.edu.cn/assets/courseware/v1/107132800f5d2734fbaca9e59357eb75/asset-v1:BUAA+B3I062270+2022_SPRING+type@asset+block/lab1-elf-1.jpg)

主要理解secton header table是编译链接的视角，而program header table是执行程序时的视角

### 操作系统启动流程

![boot](https://github.com/VOIDMalkuth/BUAA_OS_2019_Code/raw/master/%E5%AE%9E%E9%AA%8C%E6%8A%A5%E5%91%8A/Lab1%20Report/proc0.png)

## 体会与感想

* 首先是课下，看了很久的指导书，总共画了大概5，6个小时才完成课下的全部内容，主要的感觉是知识点过于零散，不易串联起来，以及各种地址感觉有点晕，同时对于官方的代码刚开始不是很看的懂（好多代码都要跨文件进行阅读，有的也看得一知半解）
* 然后是课上，第一次课上由于我宏定义``4kb``定义错了，``Extra``到最后也只有``40``分，体验极差，第二次上机，除了被最后的``my_cal.c``将十进制转为二进制时没有注意要特判``0``卡了好久，感觉不错
* 最后总结一下就是课下的知识点感觉自己其实掌握的不是很好，学习体验极差，课上就是要注意细节，心态要好（不然就像我第一次一样暴毙。。。）就比较简单了，总体而言就是感觉没有学到什么主干知识（都很零散），``lab1``就结束了

## 指导书反馈

* 希望可以再详细讲讲内存布局以及有些宏定义，不然代码看得真的极度费劲