请你对现有的物理内存管理机制进行修改，对 MOS 中$ 64 MB $物理内存的高地址 $ 32 MB $建立伙伴
系统。下面对本题中所需要实现的伙伴系统进行描述：

### 内存区间的初始化

伙伴系统将高地址$ 32 MB $划分为数个内存区间，每个内存区间有两种状态：**已分配**和**未分配**。

每个内存区间的大小只可能是$ 4 \times 2^i KB $，其中$ i $是整数且$ 0 \le i \le 10 $。

初始，共有$ 8 $个$ 4 MB $大小的内存区间，状态均为**未分配**。

### 内存区间的分配

每次通过伙伴系统分配 $ x B $的空间时，找到满足如下三个条件的内存区间：

- 该内存区间的状态为**未分配**。
- 其大小不小于 $x B $。
- 满足上面两个条件的前提下，该内存区间的起始地址最小。

如果不存在这样的内存区间，则本次分配失败；否则，执行如下步骤：

1. 设该内存区间的大小为 $ y B $，若$ \frac{y}{2} < x $或$ y = 4 K$，则将该内存区间的状态设为**已分配**，将该内存区间分配并结束此次分配过程。
2. 否则，将该内存区间分裂成两个大小相等的内存区间，状态均为**未分配**。
3. 继续选择起始地址更小的那个内存区间，并返回步骤 1。

### 内存区间的释放

当一个内存区间使用完毕，通过伙伴系统释放时，将其状态设为**未分配**。
我们称两个内存区间$ x $和$ y $是**可合并**的，当且仅当它们满足如下四个条件：

1. $ x $和$ y $的状态均为**未分配**。
2. $ x $和$ y $是由**同一个**内存区间**一次分裂**所产生的两个内存区间。
   若存在两个**可合并**的内存区间，则将两个内存区间合并，若合并后仍存在两个**可合并**的内存区间，则继续合并，直到不存在两个**可合并**的内存区间为止。

请你实现如下的三个函数：

### 初始化函数 `buddy_init`

- 函数原型为： `void buddy_init(void)`
- 调用此函数后，为 MOS 中 $ 64 MB $物理内存的高地址$ 32 MB $初始化伙伴系统。初始化结束后，伙伴系统中仅有只有$ 8 $个$ 4 MB $的待分配内存区间。

### 分配函数 `buddy_alloc`

- 函数原型为：`int buddy_alloc(u_int size, u_int *pa, u_char *pi)`
- 调用此函数后，通过伙伴系统分配大小不小于`size`字节的空间，分配逻辑见上述描述。
  如果分配失败，返回$ −1$。否则，将`pa`指向所分配内存区间的起始地址，设所分配内存区间的大小为$ 4 \times 2^i KB $，令`*pi = i`，并返回$ 0 $。
  ，令`*pi = i`，并返回$ 0 $。

### 释放函数 `buddy_free`

- 函数原型为：`void buddy_free(u_int pa)`
- 调用此函数后，通过伙伴系统释放一个状态为已分配的内存区间，其起始地址为`pa` 。释放后的合并逻辑见上述描述。

### 注意事项

你需要先在`include/pmap.h`中加入如下三个函数定义：

```c
void buddy_init(void);
int buddy_alloc(u_int size, u_int *pa, u_char *pi); 
void buddy_free(u_int pa);
```

之后再在`mm/pmap.c`中实现这三个函数。

### 评测逻辑

评测过程中，我们会将所有的`Makefile`文件、`include.mk`以及 `init/init.c`替换为 lab2 初始配置，接着将`init/init.c`中的`mips_init`函数改为如下形式：

```c
void mips_init(){
mips_detect_memory(); 
mips_vm_init();
page_init(); 
buddy_init(); 
buddy_test();
*((volatile char*)(0xB0000010)) = 0; 
}
```

最后的`*((volatile char*)(0xB0000010)) = 0;`会终止 Gxemul 模拟器的运行，避免占用评测资源。

`buddy_test`在评测过程中新添加到`init/init.c`的函数，其中仅包含以下两种操作：

- 调用`buddy_alloc`函数，我们保证`size`不为$ 0 $。
- 调用`buddy_free`函数，我们保证`pa`是之前某次调用`buddy_alloc`所得到的。

每调用一个函数算一次操作，我们保证总操作数不超过$ 1000 $。

运行`make`指令的最大时间为$ 10 $秒，运行Gxemul模拟器的最大时间为$ 4 $秒。

设伙伴系统管理的物理页数为$ n $，标程中`buddy_alloc`和`buddy_free`两个函数的时间复杂度均为$ O(n) $，请你尽量以此复杂度设计算法。

送两组样例，不送正确结果:

```c
static void buddy_test(){
    u_int pa_1, pa_2;
    u_char pi_1, pi_2;
    buddy_alloc(1572864, &pa_1, &pi_1);
    buddy_alloc(1048576, &pa_2, &pi_2);
    printf("%x\n%d\n%x\n%d\n", pa_1, (int)pi_1, pa_2, (int)pi_2);
    buddy_free(pa_1);
    buddy_free(pa_2);
}
```

```c
static void buddy_test(){
    u_int pa[10];
    u_char pi;
    int i;
    for(i = 0;i <= 9;i++){
    	buddy_alloc(4096 * (1 << i), &pa[i], &pi);
    	printf("%x %d\n", pa[i], (int)pi);
    }
    for(i = 0;i <= 9;i += 2) buddy_free(pa[i]);
    for(i = 0;i <= 9;i += 2){
    	buddy_alloc(4096 * (1 << i) + 1, &pa[i], &pi);
   		printf("%x %d\n", pa[i], (int)pi);
    }
    for(i = 1;i <= 9;i += 2) buddy_free(pa[i]);
    for(i = 1;i <= 9;i += 2){
    	buddy_alloc(4096 * (1 << i) + 1, &pa[i], &pi);
    	printf("%x %d\n", pa[i], (int)pi);
    }
    for(i = 0;i <= 9;i++) buddy_free(pa[i]);
    printf("%d\n", buddy_alloc(4096 * 1024, &pa[0], &pi));
    printf("%d\n", buddy_alloc(4096 * 1024 + 1, &pa[0], &pi));
}

```

