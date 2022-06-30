# Exam

env.h

```c
struct Env {
    // ...
    u_int env_asid;
    // ...
};
// ...
u_int exam_env_run(struct Env *e);
void exam_env_free(struct Env *e);
```

env.c

```c
u_int sys_asid = 0x4;
// ...
u_int mkenvid(struct Env *e) {
	u_int idx = (u_int)e - (u_int)envs;
	idx /= sizeof(struct Env);
	return (1 << (LOG2NENV)) | idx;
}
// ...

void
env_init(void) {
    // LIST_INIT(&env_sched_list[1]);
	sys_asid = 0x4;
	asid_bitmap[0] = asid_bitmap[1] = 0;
	// for (i = NENV - 1; i >= 0; i--) {
}

void exam_env_free(struct Env *e) {
	u_int asid_v = (e->env_asid) >> 6, asid_i = (e->env_asid) & 0x3f;
	if (asid_v == sys_asid) asid_free(asid_i);
}

u_int exam_env_run(struct Env *e) {
	u_int asid_v = (e->env_asid) >> 6, asid_i = (e->env_asid) & 0x3f;
	if (asid_v == sys_asid) return 0;
	else {
		if (asid_bitmap[asid_i >> 5] & (1 << (asid_i & 31))) {
			if (!(asid_bitmap[0] == 0xffffffff && asid_bitmap[1] == 0xffffffff)) {
				e->env_asid = (sys_asid << 6) | asid_alloc();
				return 0;
			} else {
				sys_asid += 1;
				asid_bitmap[0] = asid_bitmap[1] = 0;
				e->env_asid = (sys_asid << 6) | asid_alloc();
				return 1;
			}
		} else {
			asid_bitmap[asid_i >> 5] |= (1 << (asid_i & 31));
			e->env_asid &= 0x3f;
			(e->env_asid) |= (sys_asid << 6);
			return 0;
		}
	}
}
```

# Extra

注意最好就用静态数组，如果要在 ``struct Env``里面存太多东西，那么它在内存中就会覆盖到操作系统页表所在位置

env.h

```c
struct Env {
	// ...
    u_int env_s[3], env_w;
    // ..
};

// ...
void S_init(int s, int num);
int P(struct Env* e, int s);
int V(struct Env* e, int s);
int get_status(struct Env* e);
int my_env_create();
```

env.c

```c
u_int S[3];
#define size_tot 1000
u_int w1[1024], w2[1024], f[3], l[3];
u_int *tot[3] = {0, w1, w2};

void S_init(int s, int num) {
	S[s] = num;
}

int P(struct Env* e, int s) {
	if (e->env_w) return -1;
	if (S[s] > 0) {
		e->env_s[s] += 1;
		S[s] -= 1;
	} else {
		e->env_w = 1;
		l[s] = (l[s] + 1) % size_tot;
		tot[s][l[s]] = e->env_id;
	}
	return 0;
}

int V(struct Env *e, int s) {
	if (e->env_w) return -1;
	if (e->env_s[s] > 0) e->env_s[s] -= 1;
	if (l[s] == f[s]) S[s] += 1;
	else {
		f[s] = (f[s] + 1) % size_tot;
		struct Env* new = envs + ENVX(tot[s][f[s]]);
		new->env_w = 0;
		new->env_s[s] += 1;
	}
	return 0;
}

int get_status(struct Env * e) {
	if (e->env_w) return 1;
	else if (e->env_s[1] > 0 || e->env_s[2] > 0) return 2;
	else return 3;
}

int my_env_create() {
    struct Env* e;
    int r;
    if ((r == env_alloc(&e, 0)) != 0) return -1;
	return e->env_id;
}

```
