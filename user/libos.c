#include "lib.h"
#include <mmu.h>
#include <env.h>
// TODO: lab4-2-Extra

void
exit(void)
{
	//close_all();
    // TODO: lab4-2-Extra
    if (env->env_parent_id != 0) {
        syscall_send_sig(env->env_parent_id, 18);
    }
	syscall_env_destroy(0);
}


struct Env *env;

void
libmain(int argc, char **argv)
{

	// set env to point at our env structure in envs[].
	env = 0;	// Your code here.
	//writef("xxxxxxxxx %x  %x  xxxxxxxxx\n",argc,(int)argv);
	int envid;
	envid = syscall_getenvid();
	envid = ENVX(envid);
	env = &envs[envid];
    // TODO: lab4-2-Extra
    syscall_set_restore_addr(envid, (int)restore);
	// call user main routine
	umain(argc, argv);
	// exit gracefully
	exit();
	//syscall_env_destroy(0);
}
