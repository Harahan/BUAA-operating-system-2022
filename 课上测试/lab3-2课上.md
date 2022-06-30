# Exam

注意在 ``env_run``前输出 ``\n``，因为 ``env_run``直接切换进程了

sched.c

```c
void sched_yield(void)
{
    static int count = 0; // remaining time slices of current env
    static int point = 0; // current env_sched_list index
    struct Env *nxt_env;
    printf("\n");
    if (curenv == NULL || --count <= 0 || curenv->env_status != ENV_RUNNABLE) {
        if (count <= 0 && curenv != NULL) {
            LIST_REMOVE(curenv, env_sched_link);
            if ((curenv->env_pri) % 2) LIST_INSERT_TAIL(&env_sched_list[(point + 1) % 3], curenv, env_sched_link);
            else LIST_INSERT_TAIL(&env_sched_list[(point + 2) % 3], curenv, env_sched_link);
        }
        while(1) {
            if (!LIST_EMPTY(&env_sched_list[point])) {
                LIST_FOREACH(nxt_env, &env_sched_list[point], env_sched_link) {
                    if (nxt_env->env_status == ENV_RUNNABLE) {
                        count = (nxt_env->env_pri) << point;
                        env_run(nxt_env);
                        return;
                    }
                }
            }
            point = (point + 1) % 3;
        }
    }
    env_run(curenv);
}
```

# Extra

* 需要仔细阅读 ``BUILD_HANDLER``这个宏，它保存现场，关闭全局中断使能，执行异常处理函数，恢复现场
* 可直接用 ``c``完成，注意考虑如果该异常的指令是位于延迟槽中，则它的地址为 ``epc+4``，同时可以直接用 ``env_tf``中的值，因为此时此刻 ``KERNEL_SP``中保存的现场就是来自它
* 注意啊，``adel``的异常值为$4$

trap.c

```c
// ...
extern void handle_adel();
// ...
void trap_init() {
    // ...
    set_except_vector(4, handle_adel);
    // ...
}
// ...
```

genex.s

```assembly
// ...
BUILD_HANDLER adel adel_handler cli
// ...
NESTED(adel_handler, 0, sp)

    // 取指令
    mfc0 k0, CP0_CAUSE
    mfc0 k1, CP0_EPC
    and k0, 0x80000000
    beq k0, zero, is_in_ds_end
    	nop
		add k1, 4
    is_in_ds_end:
		lw k2, 0(k1)

     // 执行  
	and k2, 0x8000000
    beq k2, zero, handle_lh
     	nop
   
     // lw
	handle_lw:
     	lw k2, 0(k1)
     	and k2, 0xf7ffffff
     	sw k2, 0(k1)
     	j end
    	nop
    	 
   	 // lh
     handle_lh:
     	lw k2, 0(k1)
     	and k2, 0xfbffffff
     	sw k2, 0(k1)
   
     end:
     	jr ra
     	nop
END(adel_handler)
// ...
```

或者 ``adel_handler``这样写，完全没有代码量：

trap.c

```c
// ...
void adel_handler(struct Trapframe *tf) {
    u_int *va = tf->cp0_epc;
    if (va >> 31) va++;
    *va = (((*va) << 6) >> 6) | ((((*va) >> 26) == 35) ? 33 : 32);
}
// ...
```
