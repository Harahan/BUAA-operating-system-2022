# Exam

注意需要从页表中读取物理地址，位运算后返回物理页号

fork.c

```c
int make_shared(void *va) {
    if (va >= UTOP) return -1;
    u_int v = ROUNDDOWN(va, BY2PG);
    int perm = (*vpt)[VPN(v)] & 0xfff;
    if (!(((*vpd)[VPN(v) >> 10] & PTE_V) && ((*vpt)[VPN(v)] & PTE_V))) {
        if (syscall_mem_alloc(0, v, perm | PTE_R | PTE_V) < 0) return -1;
        syscall_mem_map(0, v, 0, v, perm);
    }
    if (!((*vpt)[VPN(v)] & PTE_R)) return -1;
    syscall_mem_map(0, v, 0, v, perm | PTE_R | PTE_V | PTE_LIBRARY);
    return (*vpt)[VPN(v)] & 0xfffff000;
}
```

lib.h

```c
int make_shared(void *va);
```

# Extra

最难 ``Extra``没有之一，课下自己做都调了好久，好久。。。这个 ``extra``真的是最考操作系统本身的一个 ``extra``了，总之完全不是考 ``c``语言了，注意保护现场以及恢复现场的一些技巧，我基本上将所有关键过程都放在内核进行处理，放在用户态会比较麻烦，考虑的东西会很多，毕竟用户态的东西一切进程就丢了：

* 用静态数组存储内核中要执行信号处理函数的进程的 ``TrapFrame``
* 对于系统调用在内核态时，当前进程的现场存储在 ``KERNEL_SP``里，其它进程的现场在 ``env->env_tf``中
* 当前进程是通过系统调用进入内核的，返回地址在 ``epc``寄存器中，而其它进程是由于进程调度而被切换的，仔细阅读汇编代码会发现它们进入用户态的地址在 ``env_tf``的 ``pc``寄存器中
* 由于当执行完信号处理函数后要恢复存储在内核的静态数组中的进程上下文，当前上下文仅是为了执行信号处理函数而产生的意外，所以这里就要重回内核恢复现场，由计组知识可知当执行完该处理函数会跳至 ``ra``寄存器值所指向的地址，于是修改其值为 ``restore``函数的地址，该函数实际上就是一个恢复现场的系统调用
* 注意由于 ``fork``产生的子进程需要继承父进程的信号处理函数于是要在 ``fork``中修改（由于信号处理函数地址存储在内核空间无法直接复制给子进程），添加系统调用 ``syscall_copy_handler``
* 同样对于子进程在 ``fork``过程中还要通过系统调用设置 ``restore_addr``，而对于系统刚运行就存在的进程可以在 ``ibos.c``的 ``libmain``中进程真正开始运行前设置
* 以上就完成了编号为$15$的信号处理，接下来对于$18$的信号处理其实就是在 ``ibos.c``的 ``exit``里面如果存在父进程就向它发送就好，而对于编号为$11$的信号处理就需要在 ``pmap.c``中进行相应修改，在 ``pageout``中陷入信号处理函数回来后（此时已通过 ``restore``函数恢复陷入信号处理函数前的现场），此时仍然是在内核态，可以通过汇编 ``set_sp ``将 ``sp ``寄存器的值设在 ``KERNEL_SP ``，注意此时虽然在中断处理过程但是由于以下两个原因，所以要手动修改，然后再调用 ``ret_from_exception``返回就好了：
  * 函数 ``pageout``执行会压栈于是 ``sp ``不会指向 ``KERNEL_SP ``；
  * 这个缺页中断是上一级异常处理过程中发生的，即此中断位于异常重入的过程中，所以 ``sp``也不会指向 ``KERNEL_SP``

unistd.h

```c
// ...
#define SYS_send_sig        ((__SYSCALL_BASE ) + (15 ) )
#define SYS_set_sig_handler  ((__SYSCALL_BASE ) + (16 ) )
#define SYS_restore          ((__SYSCALL_BASE ) + (17 ) )
#define SYS_set_restore_addr ((__SYSCALL_BASE ) + (18 ) )
#define SYS_copy_handler     ((__SYSCALL_BASE ) + (19 ) )
// ...
```

syscall.s

```assembly
 	// ...
    .word sys_send_sig
    .word sys_set_sig_handler
    .word sys_restore
    .word sys_set_restore_addr
    .word sys_copy_handler
```

syscall_all.c

```c
int handlers[3 * NENV], restore_addrs[NENV];
struct Trapframe tfs[NENV];

int get_handler(u_int envid, int sig) {
    int index = ENVX(envid) * 3;
    return (sig == 11) ? index : (sig == 15) ? index + 1 : index + 2;
}

void sys_send_sig(int sysno, u_int envid, int sig) {
    struct Trapframe *old;
    struct Env* e;
    int r;
    if (envid == 0 || envid == curenv->env_id) {
        envid = curenv->env_id;
        old = (struct Trapframe*) (KERNEL_SP - sizeof(struct Trapframe));
        e = curenv;
    } else {
        if ((r = envid2env(envid, &e, 0)) < 0) return;
        old = &(e->env_tf);
    }
    int handler = handlers[get_handler(envid, sig)], restore_addr = restore_addrs[ENVX(envid)];
    if (handler == 0) {
        if (sig == 11 || sig == 15) env_destroy(e);
        else return;
    }
    memcpy(tfs + ENVX(envid), old, sizeof(struct Trapframe));
    old->regs[4] = sig; // a0
    old->regs[31] = restore_addr; // ra
    if (envid == curenv->env_id) old->cp0_epc = handler; // epc
    else old->pc = handler; // pc
}

int sys_set_sig_handler(int sysno, int sig, int handler) {
    handlers[get_handler(curenv->env_id, sig)] = handler;
}

void sys_restore(int sysno) {
    bcopy(tfs + ENVX(curenv->env_id), KERNEL_SP - sizeof(struct Trapframe), sizeof(struct Trapframe));
}

void sys_set_restore_addr(int sysno, u_int envid, int addr) {
    if (envid == 0) envid = curenv->env_id;
    restore_addrs[ENVX(envid)] = addr;
}

void sys_copy_handler(int sysno, u_int envid, u_int penvid) {
    if (envid == 0) envid = curenv->env_id;
    int i, j;
    i = get_handler(envid, 11), j = get_handler(penvid, 11);
    memcpy(handlers + i, handlers + j, sizeof(int));
    i = get_handler(envid, 15), j = get_handler(penvid, 15);
    memcpy(handlers + i, handlers + j, sizeof(int));
    i = get_handler(envid, 18), j = get_handler(penvid, 18);
    memcpy(handlers + i, handlers + j, sizeof(int));
}
```

syscall_lib.c

```c
void
syscall_send_sig(u_int envid, int sig)
{
    msyscall(SYS_send_sig, envid, sig, 0, 0, 0);
}

void
syscall_set_sig_handler(int sig, int handler)
{
    msyscall(SYS_set_sig_handler, sig, handler, 0, 0, 0);
}

void
syscall_restore()
{
    msyscall(SYS_restore, 0, 0, 0, 0, 0);
}

void
syscall_set_restore_addr(u_int envid, int addr)
{
    msyscall(SYS_set_restore_addr, envid, addr, 0, 0, 0);
}
void
syscall_copy_handler(u_int envid, u_int penvid)
{
    msyscall(SYS_copy_handler, envid, penvid, 0, 0, 0);
}
```

pmap.c

```c
extern char *KERNEL_SP;

void pageout(int va, int context)
{
	// ...
	if (va < 0x10000) {
        sys_send_sig(0, 0, 11);
        set_sp(KERNEL_SP - sizeof(struct Trapframe));
        ret_from_exception();
		// ...
	}
	// ...
}
```

tlb_asm.c

```assembly
LEAF(set_sp)
nop
        addu sp, a0, zero
        jr ra
        nop
END(set_sp)
```

fork.c

```c
fork(void)
{
	// ...
    syscall_copy_handler(newenvid, syscall_getenvid());
    syscall_set_restore_addr(newenvid, (int)restore);
	//return newenvid;
}
```

ipc.c

```c
void kill(u_int envid, int sig) {
    syscall_send_sig(envid, sig);
}

void signal(int sig, void (*handler)(int)) {
    syscall_set_sig_handler(sig, (int) handler);
}

void restore() {
    syscall_restore();
}
```

lib.h

```c
// ...
void syscall_send_sig(u_int envid, int sig);
void syscall_set_sig_handler(int sig, int handler);
void syscall_restore();
void syscall_set_restore_addr(u_int envid, int addr);
void syscall_copy_handler(u_int envid, u_int penvid);
// ...
void kill(u_int envid, int sig);
void signal(int sig, void (*handler)(int));
void restore();
```

ibos.c

```c
void
exit(void)
{
	// ...
    if (env->env_parent_id != 0) syscall_send_sig(env->env_parent_id, 18);
	// syscall_env_destroy(0);
}

libmain(int argc, char **argv)
{
	// ... env = &envs[envid];
    syscall_set_restore_addr(envid, (int)restore);
	// umain(argc, argv); ...
}
```
