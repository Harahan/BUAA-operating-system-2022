# Exam

不知道为什么只设置一个变量 ``int f = 0``不对，按道理来说进程 ``id``不会为$0$（第$10$位始终为$1$）

user/syscall_lib.c

```c
int
syscall_try_acquire_console()
{
    return msyscall(SYS_try_acquire_console, 0, 0, 0, 0, 0);
}

int
syscall_release_console()
{
    return msyscall(SYS_release_console, 0, 0, 0, 0, 0);
}
```

user/lib.h

```c
int syscall_try_acquire_console(void);
int syscall_release_console(void);
```

syscall.S

```assembly
	// ...
	.word sys_try_acquire_console
	.word sys_release_console
```

unistd.h

```c
#define SYS_try_acquire_console			((__SYSCALL_BASE ) + (15 ) )
#define SYS_release_console			((__SYSCALL_BASE ) + (16 ) )
```

syscall_all.c

```c
// ...
static int s = -1, f = 0;
// ...
void sys_putchar(int sysno, int c, int a2, int a3, int a4, int a5)
{
    if (curenv->env_id == s) {
        // printcharc((char) c);
    }
	// return ;
}
// ...
int sys_try_acquire_console(void) {
	if (f == 0) {
        s = curenv->env_id, f = 1;
        return 0;
    }
    return -1;
}

int sys_release_console(void) {
    if (f == 1) {
        s = -1, f = 0;
        return 0;
    }
    return -1;
}
```

# Extra

一定要注意控制进程的状态以及何时切换，照着题目提示写，虽然它说的很凌乱

* 当接收进程开始接收时，如果 ``env_save_list``中有缓存的发送请求，要选择其中一个处理，同时唤醒发送请求的进程，当前进程继续运行，如果没有要处理的请求就要切换进程
* 当发送进程发送时，如果接收进程已经处于接收状态那么直接发送不必缓存，反之缓存发送请求并阻塞发送进程切换进程

user/ipc.c

```c
void ipc_send(u_int whom, u_int val, u_int srcva, u_int perm) {
    syscall_ipc_can_send(whom, val, srcva, perm);
}
```

user/syscall_lib.c

```c
int syscall_ipc_can_send(u_int envid, u_int value, u_int srcva, u_int perm) {
    writef("%x: sending %d to %x\n", env->env_id, value, envid);
    msyscall(SYS_ipc_can_send, envid, value, srcva, perm, 0);
    return 0;
}
```

syscall_all.c

```c
int send(struct Env* e, u_int value, u_int id, u_int perm, u_int srcva, struct Env* f) {
    struct Page* p;
    int r;
    e->env_ipc_value = value;
    e->env_ipc_from = id;
    e->env_ipc_perm = perm;
    e->env_ipc_recving = 0;
    e->env_status = ENV_RUNNABLE;
    if (srcva != 0) {
        Pte *pte;
        p = page_lookup(f->env_pgdir, srcva, &pte);
        if (p == NULL) return -E_INVAL;
        if ((r = page_insert(e->env_pgdir, p, e->env_ipc_dstva, perm)) < 0) return r;
    }
}

typedef struct save {
    u_int recv_id, v, send_id, perm, srcva, val;
}save;

save env_save_list[2000];
int ie = 0;

void sys_ipc_recv(int sysno, u_int dstva)
{
    int r, i;
    struct Env *e;
    if (dstva >= UTOP) return;
    curenv->env_ipc_recving = 1;
    curenv->env_ipc_dstva = dstva;
    for (i = 0; i < ie; i++) {
        if (env_save_list[i].v == 1 && env_save_list[i].recv_id == curenv->env_id) {
            save* tmp = env_save_list + i;
            envid2env(tmp->send_id, &e, 0);
            send(curenv, tmp->val, tmp->send_id, tmp->perm, tmp->srcva, e);
            e->env_status = ENV_RUNNABLE;
            tmp->v = 0;
            return;
        }
    }
    curenv->env_status = ENV_NOT_RUNNABLE;
    sys_yield();
}


int sys_ipc_can_send(int sysno, u_int envid, u_int value, u_int srcva, u_int perm)
{
	int r;
	struct Env *e;
	struct Page *p;
    if (srcva >= UTOP) return -E_INVAL;
    if ((r = envid2env(envid, &e, 0)) < 0) return r;
    if (e->env_ipc_recving == 0) {
        env_save_list[ie].recv_id = envid;
        env_save_list[ie].v = 1;
        env_save_list[ie].send_id = curenv->env_id;
        env_save_list[ie].perm = perm;
        env_save_list[ie].srcva = srcva;
        env_save_list[ie++].val = value;
        curenv->env_status = ENV_NOT_RUNNABLE;
        sys_yield();
    } else return send(e, value, curenv->env_id, perm, srcva, curenv);
}
```
