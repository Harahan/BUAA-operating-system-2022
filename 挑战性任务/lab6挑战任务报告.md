# lab6挑战任务报告

**黄雨石20376156**

[toc]

## 实现及测试

### EASY

#### 后台运行

首先修改汇编代码，防止前台阻塞，挂在后台程序无法继续执行：

```c
LEAF(sys_cgetc)
1:  lb  t0, 0x90000000
    // beqz    t0, 1b
    // nop
    move    v0,t0
    jr  ra
    nop
END(sys_cgetc)
```

修改``sh.c``，用一个变量记录是否后台运行，如果是，那么``fork``让子进程``wait(i)``，父进程继续在前台执行

```c
case '&':
	hang = 1;
	break;
// ...
if(!hang) wait(r);
        else {
            pid = fork();
            if (pid == 0) {
                wait(r); // son wait for spawn_env
                // 输出结束信息
                exit();
            }
        }
```

测试则使用一个大循环挂在后台运行：

```c
#include "lib.h"
#include "sh.h"
void umain(int argc, char **argv) {
    int N = 2000000000, i;
    for (i = 0; i < N; ++i);
    fwritef(1, "\n"RED(hangtest finished)"\n");
}
```

运行结果如下：

![image-20220628131348146](C:\Users\hys\AppData\Roaming\Typora\typora-user-images\image-20220628131348146.png)

#### 实现一行多命令

使用``fork``创建一个子进程执行已经识别到的命令以及对于``output_fd``及``input_fd``重新赋值以及``wait(pid)``（不然可能由于进程跑太快阻塞控制台）

```c
case ';': // use son to execute the cmd before ;
                if ((pid = fork()) == 0) {
                    goto runit;
                }
                wait(pid);
                input_fd = output_fd = -1;
                argc = hang = rightpipe = 0;
                break;
```

运行结果：

![image-20220628132813185](C:\Users\hys\AppData\Roaming\Typora\typora-user-images\image-20220628132813185.png)

#### 实现引号支持

即识别到引号，将引号中间所有字符作为一个整体

```c
if (*s == '\"') {
        *p1 = ++s;
        while (*s && !(*s == '\"' && *(s - 1) != '\\')) ++s; // be careful \"
        *s++ = 0; *p2 = s;
        return 'w';
    }
```

运行结果：

![image-20220628133225851](C:\Users\hys\AppData\Roaming\Typora\typora-user-images\image-20220628133225851.png)

#### ``tree``

编写一个 `walk` 函数，传入路径和层级数量，递归遍历给定路径的文件。

```c
void walk(char *path, int level, int rec) {
    int fd, n;
    struct File f, *dir;
    char new[MAXPATHLEN] = {0};
    if (rec == 0) return;
    if ((fd = open(path, O_RDONLY)) < 0) user_panic("open %s: %e", path, fd);
    while ((n = readn(fd, &f, sizeof f)) == sizeof f) { // get the file names in the same path
        if (f.f_name[0]) {
            dir = &f;
            printline(' ', level * 4, 0); // make output formatting
            fwritef(1, "|-- ");
            if (dir->f_type == FTYPE_REG) fwritef(1, "%s", dir->f_name);
            else fwritef(1, LIGHT_BLUE(%s), dir->f_name);
            fwritef(1, "\n");
            if (dir->f_type == FTYPE_DIR) { // go to the dir bellow the current path
                strcpy(new, path);
                strcat(new, "/");
                strcat(new, f.f_name);
                walk(new, level + 1, rec - 1);
            }
        }
    }
}

void tree_start(char *path, u_int recursive) {
    fwritef(1, GREEN(.)" "PURPLE(%s\n), path);
    walk(path, 0, recursive);
}
```

运行结果：
![image-20220628150459867](C:\Users\hys\AppData\Roaming\Typora\typora-user-images\image-20220628150459867.png)

#### ``mkdir``

通过增加文件系统的``create``服务即``fsipc_create``实现，**同时支持如果路径不存在沿途创建**，关键代码如下

```c
// serve.c
void serve_create(u_int envid, struct Fsreq_create *rq) {
    struct File *file;
    int r;
    char path[MAXPATHLEN];
    user_bcopy(rq->req_path, path, MAXPATHLEN);
    path[MAXPATHLEN - 1] = '\0';
    r = file_create(path, &file);
    file->f_size = 0;
    file->f_type = rq->req_type;
    ipc_send(envid, r, 0, 0);
}

// fs.c
int
_file_create(char *path, struct File **file)
{
	char name[MAXNAMELEN];
	int r;
	struct File *dir, *f;

	if ((r = walk_path(path, &dir, &f, name)) == 0) {
		return -E_FILE_EXISTS;
	}
	if (r != -E_NOT_FOUND || dir == 0) {
		return r;
	}

	if (dir_alloc_file(dir, &f) < 0) {
		return r;
	}

	strcpy((char *)f->f_name, name);
	*file = f;
	return 0;
}

int file_create(char *path, struct File **file) { // create the file when walk the path
    int r;
    char *p1 = path[0] == '/' ? path + 1 : 0;
    while (p1 && *p1) {
        while (*p1 && *p1 != '/') ++p1;
        if (!*p1 || !*(p1 + 1)) break;
        *p1 = '\0';
        struct File *dir;
        r = _file_create(path, &dir);
        if (r < 0 && r != -E_FILE_EXISTS) return r;
        else if (r == -E_FILE_EXISTS) file_open(path, &dir); // remember to open the file if existed, or there is a NULL pointer
        *p1++ = '/';
        dir->f_type = FTYPE_DIR;
    }
    return _file_create(path, file);
}
```

运行结果（接上一幅图）：

![image-20220628150618936](C:\Users\hys\AppData\Roaming\Typora\typora-user-images\image-20220628150618936.png)

#### ``touch``

和``mkdir``一样只是创建文件类型不同

运行结果如下：

![image-20220628134347984](C:\Users\hys\AppData\Roaming\Typora\typora-user-images\image-20220628134347984.png)

### NORMAL

#### ``history``

* 读取写入历史文件接口，支持``Up``和``Down``每次在``readline``第一行提前将``.history``内容读入一个二维数组，同时在``runcmd``开始将``buf``内容``save``，同时注意由于是追加所以添加了``O_APP``，在每次打开文件时检查该权限然后给文件偏移量赋值，具体已在``lab5-2-exam``实现
* ``history``则就是简单的打印``.history``内容

```c
#include "lib.h"
void history_init() {
    int r;
    if ((r = open("/.history", O_RDWR)) < 0) { // can't be created twice, or too low
        if ((r = create("/.history", FTYPE_REG)) < 0) user_panic("init .history failed: %d.\n", r);
    } else return;
}

void history_save(char *s) {
    int r;
    if ((r = open("/.history", O_RDWR | O_CREAT | O_APP)) < 0) user_panic("open .history failed");
    fwritef(r, s); // '\0' doesn't write to .history
    fwritef(r, "\n");
    close(r);
}

int history_read(char (*cmd)[128]) { // used char cmd[][] to save the cmd in .history
    int r, fd, cur = 1, i = 0, cmdi;
    char buf[128 * 128];
    if ((fd = open("/.history", O_RDONLY)) < 0) user_panic("open .history failed");
    if ((r = read(fd, buf, (long) sizeof buf)) < 0) user_panic("read .history failed");
    close(fd);
    while(buf[i]) {
        int cmdj = 0;
        while(buf[i] && buf[i] != '\n') cmd[cmdi][cmdj++] = buf[i++];
        if (!buf[i]) break;
        ++i; ++cmdi;
    }
    return cmdi;
}
```

* 其中``Up``和``Down``则是在``readline``时检测，以``Up``为例就是当检测到``Up``时就输出``Down``覆盖``Up``，然后就用``\b \b``去覆盖该行，最后将下标为`cmd_i`的``cmd``二维数组中的内容写到控制台然后用它更新``buf``数组

```c
if (i >= 2 && buf[i -2] == 27 && buf[i - 1] == 91 && buf[i] == 65) { // Up arrow key
    writef("%c%c%c", 27, 91, 66);
    for (i -= 2; i; --i) writef("\b \b");
    if (cmdi) strcpy(buf, cmds[--cmdi]);
    else strcpy(buf, cmds[cmdi]);
    writef("%s", buf);
    i = strlen(buf) - 1;
} else if (i >= 2 && buf[i - 2] == 27 && buf[i - 1] == 91 && buf[i] == 66) { // Down arrow key
    // writef("%c%c%c", 27, 91, 65);
    if (cmdi < cmdn - 1) {
        for (i -= 2; i; --i) writef("\b \b");
        strcpy(buf, cmds[++cmdi]);
        writef("%s", buf);
    } else buf[i - 2] = buf[i - 1] = buf[i] = '\0';
    i = strlen(buf) - 1;
}
// ...
```

运行结果：

![image-20220628140003190](C:\Users\hys\AppData\Roaming\Typora\typora-user-images\image-20220628140003190.png)

### CHALLENGE

#### 环境变量

使用链表存储变量``name``、``value``、``readonly``、``visibility``、``valid``，注意每个进程均有一个变量列表，但是变量节点一共只有$64$个：

```c
struct var {
    char name[64], values[64];
    int readonly, v, vis;
    LIST_ENTRY(var) var_link;
};
LIST_HEAD(var_list, var);
struct var_list local_vars[1024];
struct var vars[64];
```

由于要操作的不是当前进程的变量，而是当前进程所属``shell``的变量，同时每个``shell``都经过``spawn``创建，因此对`spawn`进行修改，即提供系统调用将进程控制块中的``env_is_shell``（自己添加）置$1$，同时要让当前指令知道所属``shell``因此只要调用一次``spawn``就要设置``env_parent_id``为创建当前指令的进程的``id``（对于``fork``的进程已经设置，所以不用管），同时注意在``env_destory``中清空``env_is_shell``，``env_parent_id``

```c
// spawn.c
envs[ENVX(child_envid)].env_parent_id = syscall_getenvid();
if (strcmp(prog, "sh.b") == 0) {
    envs[ENVX(child_envid)].env_is_shell = 1;
    syscall_env_inherit_var(child_envid);
}
```

于是通过下面函数可以获得命令所属``shell``：

```c
int get_shell_id(u_int envid) {
    struct Env *env = &(envs[ENVX(envid)]);
    while(env->env_is_shell != 1) env = &(envs[ENVX(env->env_parent_id)]);
    return env->env_id;
}
```

同时如果当前指令是``sh``，那么也要通过系统调用让它去继承父``shell``所拥有的环境变量，也在``spawn``中进行系统调用：

```c
void sys_env_inherit_var(int sysno, u_int envid) {
    struct var_list *p_list = &(local_vars[ENVX(get_shell_id(envs[ENVX(envid)].env_parent_id))]),
            *s_list = &(local_vars[ENVX(envid)]);
    struct var *tmp, *s;
    LIST_FOREACH(tmp, p_list, var_link) {
        if (tmp->vis == 0) {
            s = get_var();
            memcpy(s, tmp, sizeof(struct var));
            LIST_INSERT_HEAD(s_list, s, var_link);
        }
    }
}
```

接下来提供操作``shell``变量的系统调用``syscall_env_var``，具体就是对链表的一系列操作，从而维护每个``shell``的链表：

* ``create``：当前``shell``创建一个变量，如果有则根据权限覆盖，``type``则用于判断是否创建还是仅覆盖
* ``get``：返回当前``shell``名为``name``的变量值
* ``unset``：根据权限是否取消当前``shell``的变量
* ``check``：检查当前``shell``是否存在名为``name``的变量
* ``get_list``：获取当前``shell``的所有变量

$tips:$``get_var``为在所有的变量节点中寻找一个无效的即``v``为$0$的节点

```c
int create(const char *name, const char *value, const u_int vis, const u_int readonly, const int type) {
    struct var_list *temp_list = &(local_vars[ENVX(get_shell_id(curenv->env_id))]);
    struct var *tmp;
    LIST_FOREACH(tmp, temp_list, var_link) {
        if (strcmp(tmp->name, name) == 0 && tmp->readonly) {
            return -E_ENV_VAR_READONLY;
        } else if (strcmp(tmp->name, name) == 0) {
            strcpy(tmp->values, value); tmp->vis = vis; tmp->readonly = readonly;
            return 0;
        }
    }
    if (type) return -E_ENV_VAR_NOT_FOUND;
    tmp = get_var();
    strcpy(tmp->values, value); tmp->vis = vis; tmp->readonly = readonly;
    strcpy(tmp->name, name); tmp->v = 1;
    LIST_INSERT_HEAD(temp_list, tmp, var_link);
    return 0;
}

int get(char *name, char *value) {
    struct var_list *temp_list = &(local_vars[ENVX(get_shell_id(curenv->env_id))]);
    struct var *tmp;
   	LIST_FOREACH(tmp, temp_list, var_link) {
        if (strcmp(tmp->name, name) == 0) {
            strcpy(value, tmp->values);
            return 0;
        }
    }
    return -E_ENV_VAR_NOT_FOUND;
}

int unset(char *name) {
    struct var_list *temp_list = &(local_vars[ENVX(get_shell_id(curenv->env_id))]);
    struct var *tmp;
    LIST_FOREACH(tmp, temp_list, var_link) {
        if (strcmp(tmp->name, name) == 0 && tmp->readonly == 0) {
            LIST_REMOVE(tmp, var_link);
            tmp->v = 0;
            return 0;
        } else if (strcmp(tmp->name, name) == 0) {
            return -E_ENV_VAR_READONLY;
        }
    }
    return -E_ENV_VAR_NOT_FOUND;
}

int check(char* name) {
    struct var_list *temp_list = &(local_vars[ENVX(get_shell_id(curenv->env_id))]);
    struct var *tmp;
    LIST_FOREACH(tmp, temp_list, var_link) {
        if (strcmp(tmp->name, name) == 0) return 0;
    }
    return -E_ENV_VAR_NOT_FOUND;
}

int get_list(char *name, char *value) {
    struct var_list *temp_list = &(local_vars[ENVX(get_shell_id(curenv->env_id))]);
    struct var *tmp;
    int pos = 0;
    char **name_list = (char**)name, **value_list = (char**)value;
    LIST_FOREACH(tmp, temp_list, var_link) {
        name_list[pos] = tmp->name;
        value_list[pos++] = tmp->values;
    }
    name_list[pos] = 0;
    return 0;
}

int sys_env_var(int sysno, char *name, char *value, u_int vis, u_int readonly, u_int op) {
    switch (op) {
        case 0:
            return create(name, value, vis, readonly, 0);
        case 1:
            return get(name, value);
        case 2:
            return create(name, value, vis, readonly, 1);
        case 3:
            return unset(name);
        case 4:
            return get_list(name, value);
        case 5:
            return check(name);
        default:
            return 0;
    }
}
```

``declare``如果有参数则调用用``create``否则调用``get_list``，而``unset``就是调用``unset``，``echo``则调用``get``

运行结果：

* ``echo``：

  ![image-20220628145459778](C:\Users\hys\AppData\Roaming\Typora\typora-user-images\image-20220628145459778.png)

* ``declare``：

  ![image-20220628145649883](C:\Users\hys\AppData\Roaming\Typora\typora-user-images\image-20220628145649883.png)

* ``unset``：

  ![image-20220628145842132](C:\Users\hys\AppData\Roaming\Typora\typora-user-images\image-20220628145842132.png)

  ![image-20220628145944256](C:\Users\hys\AppData\Roaming\Typora\typora-user-images\image-20220628145944256.png)

除第一幅图外其余三幅图为连续的，由于每个``shell``一个链表所以它们之间的变量是无关的，只有在新建时会继承环境变量

## 额外shell命令实现

### ``>>``和``O_CREATE``

由于加了``O_APP``，故只要在解析命令时解析就好，而``O_CREATE``则直接照搬课上测试代码即可：

```c
if (*s == '>' && *(s + 1) == '>') {
        *p1 = s;
        *s++ = 0;
        *s++ = 0;
        *p2 = s;
        return 'a';
    }
```

运行结果：

![image-20220629162857211](C:\Users\hys\AppData\Roaming\Typora\typora-user-images\image-20220629162857211.png)

### ``cd``

直接在``shell``刚开始运行时添加环境变量``curpath``，此后其余所有命令如果第一个字符不是``/``，则在当前目录下进行，同时``cd``就是设置环境变量为指定路径，并且支持``cd .``以及``cd ..``，可以仿造``linux``系统在``$``前输出当前路径：

接口如下：

```c
const char *CURPATH_KEY = "curpath";
void curpath_init(char *path) {
    int r;
    if (syscall_env_var(CURPATH_KEY, path, 0, 0, 5) != 0
    && (r = syscall_env_var(CURPATH_KEY, path, 0, 0, 0)) < 0) user_panic("Init curpath failed: %d", r);
}

int curpath_get(char *path) {
    int r;
    if ((r = syscall_env_var(CURPATH_KEY, path, 0, 0, 1)) < 0) return r;
}

int curpath_set(char *path) {
    int r;
    if ((r = syscall_env_var(CURPATH_KEY, path, 0, 0, 2)) < 0) return r;
}

int curpath_get_parent(char *path) {
    int r, i;
    if ((r = curpath_get(path)) < 0) return r;
    if (strlen(path) == 1) return 0;
    for (i = strlen(path) - 2; path[i - 1] != '/'; i--);
    path[i] = 0;
}
```

运行结果可以见之前``tree``的运行截图可体现

### ``clear``

直接输出``x1b[2Jx1b[H``即可

### 彩色输出

通过宏定义实现：

```c
/* Define color modes */
#define BOLD_GREEN(str) "\033[0;32;32m\033[1m" # str "\033[m"
#define RED(str) "\033[0;32;31m" # str "\033[m"
#define LIGHT_RED(str) "\033[1;31m" # str "\033[m"
#define GREEN(str) "\033[0;32;32m" # str "\033[m"
#define LIGHT_GREEN(str) "\033[1;32m" # str "\033[m"
#define BLUE(str) "\033[0;32;34m" # str "\033[m"
#define LIGHT_BLUE(str) "\033[1;34m" # str "\033[m"
#define DARK_GRAY(str) "\033[1;30m" # str "\033[m"
#define CYAN(str) "\033[0;36m" # str "\033[m"
#define LIGHT_CYAN(str) "\033[1;36m" # str "\033[m"
#define PURPLE(str) "\033[0;35m" # str "\033[m"
#define LIGHT_PURPLE(str) "\033[1;35m" # str "\033[m"
#define BROWN(str) "\033[0;33m" # str "\033[m"
#define YELLOW(str) "\033[1;33m" # str "\033[m"
#define LIGHT_GRAY(str) "\033[0;37m" # str "\033[m"
#define WHITE(str) "\033[1;37m" # str "\033[m"
```

### ``rm``

直接调用``remove``函数

运行结果：

![image-20220628151508443](C:\Users\hys\AppData\Roaming\Typora\typora-user-images\image-20220628151508443.png)

### ``exist``退出当前``shell``

由于记录了当前指令所属的``env_is_shell``故该命令直接通过一个系统调用将当前``shell``给``destroy``，注意要清空当前``shell``变量并将节点设为无效：

```c
u_int sys_env_get_shell(int sysno) {
    u_int pid = get_shell_id(curenv->env_id);
    struct var_list *temp_list = &(local_vars[ENVX(pid)]);
    struct var *tmp = LIST_FIRST(temp_list), *pre;
    while(!LIST_EMPTY(temp_list)) {
        pre = tmp;
        tmp = LIST_NEXT(tmp, var_link);
        LIST_REMOVE(pre, var_link);
        pre->v = 0;
    }
    env_destroy(envs + ENVX(pid));
    return pid;
}
```

运行结果见环境变量最后一幅图

## 难点与解决方案

* 存储``shell``变量的链表开太大导致硬件直接报错，``de``了很久
* 如何让当前命令知道当前``shell``以及如何使子``shell``继承父``shell``环境变量，解决方案如上
* 如何存储变量，主要是没有看懂``declare``和``unset``的具体行为对于父``shell``和子``shell``影响，第一遍行为和``linux``完全不同，第二遍在询问助教以及自己在命令行尝试后解决，除了``declare -x``会使子``shell``无法继承，其余都会被继承，同时改变当前``shell``的变量不会影响别的进程，相当于当前``shell``的变量是父进程的环境变量的副本以及自己定义的变量的集合
* 创建文件时沿途寻找路径没有办法在已经有的路径下创建，主要原因是``file_create``（即我改名后的``_file_create``）当发现有该文件存在时将提供的文件指针赋值为空，而继续往下寻找则为找空指针，故当发现已经存在路径时，不能接着继续找，要先``file_open``为文件指针赋值
* ``;``那里发现如果不``wait(pid)``控制台非常容易阻塞，及执行完``xxx;xxx``后无法输入，可能是多个进程对一个文件操作导致阻塞了

总体来说每个命令都不简单写完后都调了好一会儿，特别是环境变量那里，操作很繁琐

