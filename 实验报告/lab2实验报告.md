# lab2实验报告

**黄雨石20376156**

[toc]

## 实验思考题

### Thinking 2.1
**请你根据上述说明，回答问题:**

- **在我们编写的程序中，指针变量中存储的地址是虚拟地址还是物理地址？**
- **MIPS 汇编程序中lw, sw使用的是虚拟地址还是物理地址？**

**answer：**指针变量存储的是虚拟地址，MIPS汇编程序中使用的也是虚拟地址。因为实验使用的R3000 CPU只会发出虚拟地址，然后虚拟地址映射到物理地址，使用物理地址进行访存。

### Thinking 2.2

- **请从可重用性的角度，阐述用宏来实现链表的好处。**
- **请你查看实验环境中的 /usr/include/sys/queue.h，了解其中单向链表与循环链表的实现，比较它们与本实验中使用的双向链表，分析三者在插入与删除操作上的性能差异。**

**answer：1)**宏的一个本身的特性就是可重用，跟函数一样，可以将一段代码封装成一条语句。当这段代码的具体实现需要更改时，只需要改宏这一处就行。宏相比函数也更加轻便，可以用于结构体定义等，由于是**字符串的替换**，因此不必进行地址的跳转和栈的保存，但值得注意的是在编写宏的时候需要着重注意语法是否有漏洞。此外``do/while(0)``的架构也大大方便了调用这些宏，可以直接将其当做函数看待。**2)插入操作：**单向链表插入操作十分简单，两行代码，双向链表插入操作一般运行四行代码，需要额外判断是否``next``指向了``NULL``，循环链表与双向链表运行代码量基本相等，需额外判断是否``next``指向了头指针。特别的是，插入到头结点对三种链表而言性能相似，单向链表与双向链表插入到尾结点均要遍历完整个链表。**删除操作：**单向链表的删除操作复杂度为``O(n)``，因为需要靠循环才能找到上一个链表节点的位置，双向链表及循环链表的删除操作与插入性能相近，也还是需要额外判断``NULL``或``HEAD``。删除头结点对三种链表而言性能相似，而单向链表与双向链表删除尾结点还是要遍历。

### Thinking 2.3

**请阅读 `include/queue.h` 以及 `include/pmap.h`, 将 `Page_list` 的结构梳理清楚，选择正确的展开结构。**

```c
A:
struct Page_list{
    struct {
        struct {
            struct Page *le_next;
            struct Page **le_prev;
        }* pp_link;
        u_short pp_ref;
    }* lh_first;
}

```



```c
B:
struct Page_list{
    struct {
        struct {
            struct Page *le_next;
            struct Page **le_prev;
        } pp_link;
        u_short pp_ref;
    } lh_first;
}
```



```c
C:
struct Page_list{
    struct {
        struct {
            struct Page *le_next;
            struct Page **le_prev;
        } pp_link;
        u_short pp_ref;
    }* lh_first;
}
```

**answer:**

```verilog
 typedef LIST_ENTRY(Page) Page_LIST_entry_t;
 
 struct Page {
     Page_LIST_entry_t pp_link;    /* free list link */
 
     // Ref is the count of pointers (usually in page table entries)
     // to this page.  This only holds for pages allocated using
     // page_alloc.  Pages allocated at boot time using pmap.c's "alloc"
     // do not have valid reference count fields.
 
     u_short pp_ref;
 };
 
 #define LIST_HEAD(name, type)                                               \
         struct name {                                                           \
                 struct type *lh_first;  /* first element */                     \
         }
 
 #define LIST_ENTRY(type)                                                    \
         struct {                                                                \
                 struct type *le_next;   /* next element */                      \
                 struct type **le_prev;  /* address of previous next element */  \
         }
 
```

答案选C。``Page_list``中含有的是``Page``结构体指针头。每一个``Page``内存控制块都有一个`pp_ref`用于表示其引用次数（为0时便可remove)，还有一个结构体用于存放实现双向链表的指针。

### Thinking 2.4

**请你寻找上述两个 boot_\* 函数在何处被调用。**

**answer:**

```cpp
 //在boot_map_segment()函数中调用到了boot_pgdir_walk()函数
 //以此得到虚拟地址所对应的二级页表项
 pgtable_entry = boot_pgdir_walk(pgdir, va_temp, 1); //create 
 
 //在mips_vm_init()函数中调用到了boot_map_segment函数
 boot_map_segment(pgdir, UPAGES, n, PADDR(pages), PTE_R);
 boot_map_segment(pgdir, UENVS, n, PADDR(envs), PTE_R);
 //alloc已经分配好了虚拟地址
 //boot_map_segment分别将页面结构体与进程控制块结构体的虚拟地址映射成物理地址
```

### Thinking 2.5

- **请阅读上面有关 R3000-TLB 的叙述，从虚拟内存的实现角度，阐述 ASID 的必要性**
- **请阅读《IDT R30xx Family Software Reference Manual》的 Chapter 6，结合 ASID 段的位数，说明 R3000 中可容纳不同的地址空间的最大数量**

**answer:1)**同一虚拟地址在不同地址空间中通常映射到不同物理地址，ASID可以判断是在哪个地址空间。例如有多个进程都用到了这个虚拟地址，但若该虚拟地址对应的数据不是共享的，则基本可以表明指向的是不同物理地址，这也是一种对地址空间的保护。**2)**64个，参考原文如下：

> Instead, the OS assigns a 6-bit unique code to each task’s distinct address space. Since the ASID is only 6 bits long, OS software does have to lend a hand if there are ever more than **64 address spaces** in concurrent use; but it probably won’t happen too often.

### Thinking 2.6 

**请你完成如下三个任务：**

- **tlb_invalidate 和 tlb_out 的调用关系是怎样的？**
- **请用一句话概括 tlb_invalidate 的作用**
- **逐行解释 tlb_out 中的汇编代码**

**answer:1)**``tlb_invalidate``调用``tlb_out``**2）**调用``tlb_invalidate``可以将该地址空间的虚拟地址对应的表项清除出去，一般用于这个虚拟空间引用次数为``0``时释放``tlb``空间**3)**

```assembly
 LEAF(tlb_out)
 //1: j 1b
 nop
     mfc0    k1,CP0_ENTRYHI  //保存ENTRIHI原有值
     mtc0    a0,CP0_ENTRYHI  //将传进来的参数(虚页号)放进ENTRYHI中
     nop
     tlbp// insert tlbp or tlbwi //检测ENTRYHI中的虚拟地址在tlb中是否有对应项
     nop
     nop
     nop
     nop
     mfc0    k0,CP0_INDEX    //INDEX可以用来判断是否命中
     bltz    k0,NOFOUND  //若未命中，则跳转
     nop
     mtc0    zero,CP0_ENTRYHI    //将ENTRYHI清零
     mtc0    zero,CP0_ENTRYLO0   //将ENTRYLO清零
     nop
     tlbwi// insert tlbp or tlbwi    //将清零后的两寄存器值写入到对应tlb表项中
                                     //相当于删除原有的tlb表项
 NOFOUND:
 
     mtc0    k1,CP0_ENTRYHI  //将原来的ENTRYHI恢复
     
     j   ra  //return address
     nop
 END(tlb_out)
```

### Thinking 2.7

**在现代的 64 位系统中，提供了 64 位的字长，但实际上不是 64 位页式存储系统。假设在 64 位系统中采用三级页表机制，页面大小 4KB。由于 64 位系统中字长为 8B，且页目录也占用一页，因此页目录中有 512 个页目录项，因此每级页表都需要 9 位。因此在 64 位系统下，总共需要 3 × 9 + 12 = 39 位就可以实现三级页表机制，并不需要 64 位。现考虑上述 39 位的三级页式存储系统，虚拟地址空间为 512 GB，若记三级页表的基地址为 PTbase ，请你计算：**

- **三级页表页目录的基地址**
- **映射到页目录自身的页目录项(自映射)**

**answer:1)**页表基地址(page table) 为$PT_{base}$，页中间目录基地址(page middle directory) $PMD_{base}$：
$$
(PT_{base} >> 12) << 3 + PT_{base}
$$
页全局目录(page global directory) $PGD_{base}$**（三级页表页目录的基地址）**：
$$
(PT_{base} >> 21) << 3 + PMD_{base}
$$
**2)**页全局目录项（page global directory entry）$PGDE$**（映射到页目录自身的页目录项）**：
$$
(PT_{base} >> 30) << 3 + PGD_{base}
$$

### Thinking 2.8

 **任选下述二者之一回答：**

- **简单了解并叙述 X86 体系结构中的内存管理机制，比较 X86 和 MIPS 在内存管理上的区别。**
- **简单了解并叙述 RISC-V 中的内存管理机制，比较 RISC-V 与 MIPS 在内存管理上的区别。**

**answer:**X86用到三个地址空间的概念：物理地址、线性地址和逻辑地址。而MIPS只有物理地址和虚拟地址两个概念且只支持分页。相对而言，段机制对大量应用程序分散地使用大内存的支持能力较弱。所以Intel公司又加入了页机制，每个页的大小是固定的（一般为4KB），也可完成对内存单元的安全保护，隔离，且可有效支持大量应用程序分散地使用大内存的情况。x86体系中，TLB表项更新能够由硬件自己主动发起，也能够有软件主动更新。

> 分段机制和分页机制都启动：逻辑地址--->**段机制处理**--->线性地址--->**页机制处理**--->物理地址

RISC-V提供三种权限模式（MSU），而MIPS只提供内核态和用户态两种权限状态。RISC-V SV39支持39位虚拟内存空间，每一页占用4KB，使用三级页表访存。

## 实验难点图示

### 物理内存

* 链表的各部分结构：

  <img src="Z:\buaa_os_2022\实验报告\os_link_list.png" style="zoom: 33%;" />

* lab2-1主要完成物理页控制块的相关初始化，且注意在内核态的页表在初始化的时候就将物理页控制块以及进程控制块通过内核页表映射到了$0x7f400000$与$0x7fc00000$之间，同时还将它们直接通过线性映射到了kseg0，主要便于同时在内核态以及用户态进行该部分内容的访问

### 虚拟内存

* 用户态视角下内存结构：

  <img src="C:\Users\hys\AppData\Roaming\Typora\typora-user-images\image-20220422193404650.png" alt="image-20220422193404650" style="zoom: 50%;" />

* 而内核态下即lab2-2的内容中，位于kseg0的内核页表并没有做自映射，与上图页表不太一样，内核态的页表在虚拟地址中也不是连续的，且由于位于内核态，kseg0的地址可以通过直接抹去高位的1确定物理地址，而自映射的主要目的其实是便于用户在用户态访问所有自身映射过的页表本身，所以在内核态做自映射并无实际意义。

## 体会与感想

* 这次的课下特别是虚拟内存部分非常有难度，具体做了多久我没有统计过，但感觉填完代码还是挺快的，不过真正对于内存相关的初始化有了一定的认识还是在反复分析代码运行流程以及和各函数的调用以及功能后才有所理解，感觉学到了很多东西，但是吧感觉自己对于``tlb``相关的操作流程理解的并不是很深刻，因为就只看过``tlb_invalidate``，``tlb_out``这两个函数
* 课上的话，第一次的Extra没有做出来，主要还是c语言好久没写了，且忘了用数组，而是用了指针但是又忘了还有``alloc``这个函数，属于是定义了指针但是不分配空间，非常愚蠢。。。

## 指导书反馈

* Exercise2.8:

  <img src="C:\Users\hys\AppData\Roaming\Typora\typora-user-images\image-20220422200606792.png" alt="image-20220422200606792" style="zoom:67%;" />

  当分配了一页后，需要将其对应的页控制块的引用加1，但是给出的框架中没用且也没叫补这行代码

