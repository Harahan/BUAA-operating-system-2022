# lab3实验报告

**黄雨石20376156**

[toc]

## 实验思考题

### Thinking3.1

**思考`envid2env` 函数:为什么`envid2env`中需要判断`e->env_id != envid` 的情况？如果没有这步判断会发生什么情况？**

**answer:**通过阅读函数代码，我们可以知道``envid``的后10位为其对应的进程块在``envs``数组中的下标

```c
e = envs + ENVX(envid);

if (e->env_status == ENV_FREE || e->env_id != envid) {
    *penv = 0;
    return -E_BAD_ENV;
}
```

但是其对应进程块当前的``env_id``不一定等于``envid``

因为`env_id`中还有`ASID`字段（11-16位），当`envs`中的某一个进程控制块被回收后又重新分配，新生成的`env_id`的后10位不变，但是`ASID`字段会改变。所以我们需要进一步判断`e->env_id != envid`是否成立。如果不判断，``envid``对应的是被回收分配前的进程块，那么该函数仍然将返回当前进程块，但实际上回收分配前的进程块和现在的进程块对应的进程已经不是同一个进程

### Thinking3.2

**结合include/mmu.h 中的地址空间布局，思考`env_setup_vm` 函数：**

- **`UTOP` 和`ULIM`的含义分别是什么，`UTOP` 和 `ULIM` 之间的区域与UTOP以下的区域相比有什么区别？**
- **请结合系统自映射机制解释代码中`pgdir[PDX(UVPT)]=env_cr3`的含义。**
- **谈谈自己对进程中物理地址和虚拟地址的理解。**

**answer:**

- `ULIM`是`kseg0`和`kuseg`的分界线，也是内核态虚拟内存区和用户态虚拟内存区的分界线；`UTOP`是`kuseg`中只读区和可读写区的分界线。**`UTOP`和`ULIM`之间是只读区域**，用来存放用户的进程信息和页表信息；**`UTOP`之下的是可读写区域**，用户可以自由读写。
- `UVPT`是`kuseg`中保存用户（当前进程）页表信息的虚拟内存区，大小为4MB（0x7fc00000-0x80000000），`env_cr3`是进程页目录所在的物理地址。`pgdir[PDX(UVPT)]=env_cr3`表示页目录中第`PDX(UVPT)`项映射到页目录本身的物理地址，实现自映射机制。
- 在用户进程中使用的是虚拟地址（用户可见），然后MMU通过查询页表得到对应的物理地址（用户不可见），并最终通过物理地址在物理内存中获取值。

### Thinking3.3

 **找到 `user_data` 这一参数的来源，思考它的作用。没有这个参数可不可以？为什么？（可以尝试说明实际的应用场景，举一个实际的库中的例子）**

**answer:**`user_data` 在 `load_elf` 和 `load_icode_mapper` 两个函数中用到。

```c
int load_elf(u_char *binary, int size, u_long *entry_point, void *user_data,
			 int (*map)(u_long va, u_int32_t sgsize, u_char *bin, u_int32_t bin_size, void *user_data)) {
    // ...
     r = map(phdr->p_vaddr, phdr->p_memsz, binary + phdr->p_offset, phdr->p_filesz, user_data);
     if (r != 0) return r;
    // ...
}

static int load_icode_mapper(u_long va, u_int32_t sgsize, u_char *bin, u_int32_t bin_size, void *user_data) {
    // ...
    struct Env *env = (struct Env *)user_data;
    // ...
}
```

不可以没有这个参数，因为在`load_elf`函数中需要进一步将这个参数传递给函数指针`map`（实际上指向的是`load_icode_mapper`）。而在``map``函数中，``user_data``其实是当前进程的控制块，需要通过它查询当前进程块页表

### Thinging3.4

**结合`load_icode_mapper` 的参数以及二进制镜像的大小，考虑该函数可能会面临哪几种复制的情况？你是否都考虑到了？**

**answer:**`va`和`va+bin_size`的相对位置有以下6种

![image-20220512173854775](C:\Users\hys\AppData\Roaming\Typora\typora-user-images\image-20220512173854775.png)

`va+bin_size`和`va+sg_size`的相对位置有以下6种

![image-20220512173823493](C:\Users\hys\AppData\Roaming\Typora\typora-user-images\image-20220512173823493.png)

### Thinking3.5

**思考上面这一段话，并根据自己在lab2 中的理解，回答：**

- **你认为这里的 `env_tf.pc` 存储的是物理地址还是虚拟地址?**
- **你觉得`entry_point`其值对于每个进程是否一样？该如何理解这种统一或不同？**

**answer:**env_tf.pc中存储的是虚拟地址。对于每个进程来说entry_point都是一样的，这样可使让程序的每次执行都从一个固定的虚拟地址开始，这种统一对CPU是友好的。

### Thinking3.6

**请查阅相关资料解释，上面提到的`epc`是什么？为什么要将`env_tf.pc`设置为`epc`呢？**

**answer:**`epc`是CP0中的一个寄存器（14号），用于保存使该进程出现异常（包括中断）时指令所在的`pc`。进程因异常被切换时将`env_tf.pc`设置为`epc`，实际上就是保存了“该进程运行到了什么地方”。这样就可以使进程在下一次被调度时，**直接从上次出现异常的位置继续向后运行**。

### Thinking3.7

**关于 `TIMESTACK`，请思考以下问题：**

- **操作系统在何时将什么内容存到了 `TIMESTACK` 区域**
- **`TIMESTACK` 和 `env_asm.S` 中所定义的 `KERNEL_SP` 的含义有何不同**

**answer:**操作系统在进程切换时将新进程的`env_tf`中的内容保存到`TIMESTACK`区域。 `TIMESTACK`时发生时钟中断时的栈顶指针，`KERNEL_SP`是发生其他中断时的栈顶指针。

### Thinking3.8

**试找出上述 5 个异常处理函数的具体实现位置**

**answer:**只能找出`handle_int`和`handle_sys`的实现位置，前者在`genex.S`文件中，后者在`syscall.S`文件中

``handle_tlb``，``handle_mod``，``handle_reserved``也在``genex.S``：

```c
BUILD_HANDLER tlb	do_refill	cli
BUILD_HANDLER mod	page_fault_handler cli
BUILD_HANDLER reserved do_reserved cli
```

### Thinking3.9

**阅读 `kclock_asm.S` 和 `genex.S` 两个文件，并尝试说出 `set_timer` 和 `timer_irq` 函数中每行汇编代码的作用**

**answer:**

* ``set_timer``：

  ```assembly
  LEAF(set_timer)
  	li t0, 0xc8							
  	sb t0, 0xb5000100					# 在地址0xb5000100中写入0xc8，使时钟频率设为200
  	sw	sp, KERNEL_SP					# 将sp寄存器中的值存入地址KERNEL_SP中
  setup_c0_status STATUS_CU0|0x1001 0       # 将CP0的SR寄存器中第1、12、28位 置为1，作用是开启全局中断使能和始终中断使能，并允许用户态使用CP0
  	jr ra								# 函数返回
  	nop
  END(set_timer)
  ```

  

* ``timer_irq``：

  ```assembly
  timer_irq:
  
  	sb zero, 0xb5000110					# 在地址0xb0000110中写入0，响应时钟中断
  1:	j	sched_yield						# 跳转到sched_yield函数，进行进程切换
  	nop
  	/*li t1, 0xff
  	lw    t0, delay
  	addu  t0, 1
  	sw	t0, delay
  	beq	t0,t1,1f	
  	nop*/
  	j	ret_from_exception				# 跳转到ret_from_exception函数
  	nop
  ```

### Thinking3.10

**阅读相关代码，思考操作系统是怎么根据时钟周期切换进程的。**

**answer:** `env_sched_list`有两个链表存放就绪进程。当进程被创建时，我们将其插入第一个进程调度链表的头部。调用 `sched_yield` 函数时, 先判断当前时间片是否用完。如果用完，则将其插入另一个链表的结尾，之后判断当前进程链表是否有就绪进程。如果没有, 将指针切换到另一个就绪状态进程链表。最后从指针指向的链表的头部获取一个就绪进程进行切换。

## 实验难点图示

我认为该实验的难点在于理解**进程的创建流程**、**时钟中断的处理流程**和**进程的切换流程**。大致流程如下

* 进程创建流程

  ```mermaid
  graph LR
  A((env_create_priority))--> B((env_alloc)) --> C((load_icoade)) ---> D((LIST_INSERT_HEAD))
  ```

  

* 时钟中断处理流程

  ```mermaid
  graph LR
  A((except_vec3)) ---> B((handle_int)) ---> C((timer_irq)) ---> D((sched_yield)) ---> E((env_run)) ---> F((ret_from_exception))
  ```

  

* 进程切换流程

  ```mermaid
  graph LR
  A((env_run)) ---> B((bcopy)) ---> C((lcontext)) ---> D((env_pop_tf))
  ```

  

## 体会与感想

* 这个``lab3``比``lab2``好填一些，前前后后大概也化了10个小时左右，主要还是难在不容易在大脑中形成一个全局的观念，特别是由于部分代码是以汇编形式给出，难以理解，于是思路很容易就中断了
* 上机而言个人感觉与课下关系不大，反而和``c``语言基础以及题目理解细心程度相挂钩



