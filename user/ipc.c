// User-level IPC library routines

#include "lib.h"
#include <mmu.h>
#include <env.h>

extern struct Env *env;
typedef struct node {
    u_int envid;
    u_int v;
    void (*handler)(int);
} node;
node arr[1000];
u_int arr_size;

// Send val to whom.  This function keeps trying until
// it succeeds.  It should panic() on any error other than
// -E_IPC_NOT_RECV.
//
// Hint: use syscall_yield() to be CPU-friendly.
void
ipc_send(u_int whom, u_int val, u_int srcva, u_int perm)
{
	int r;

	while ((r = syscall_ipc_can_send(whom, val, srcva, perm)) == -E_IPC_NOT_RECV) {
		syscall_yield();
		//writef("QQ");
	}

	if (r == 0) {
		return;
	}

	user_panic("error in ipc_send: %d", r);
}

// Receive a value.  Return the value and store the caller's envid
// in *whom.
//
// Hint: use env to discover the value and who sent it.
u_int
ipc_recv(u_int *whom, u_int dstva, u_int *perm)
{
	//printf("ipc_recv:come 0\n");
	syscall_ipc_recv(dstva);

	if (whom) {
		*whom = env->env_ipc_from;
	}

	if (perm) {
		*perm = env->env_ipc_perm;
	}

	return env->env_ipc_value;
}

void kill(u_int envid, int sig) {
    int i = syscall_kill(envid, sig, arr, arr_size);
    if (i < 0) exit();
}

static int get() {
    int i = 0;
    for (; i < arr_size; i++) {
        if (arr[i].envid == syscall_getenvid() && arr[i].v == 1) {
            return i;
        }
    }
    return -1;
}

void signal(int sig, void (*handler)(int)) {
    int i = 0;
    if (handler == NULL) {
        if ((i = get()) < 0) return;
        arr[i].v = 0;
    }
    arr[arr_size].v = 1;
    arr[arr_size].handler = handler;
    arr[arr_size++].envid = syscall_getenvid();
}

