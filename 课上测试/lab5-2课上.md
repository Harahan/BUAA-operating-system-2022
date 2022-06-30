# Exam

注意 ``O_ALONE``在 ``fork``前父子进程的文件描述符的偏移是一样的，刚 ``fork``后也一样（不需要修改 ``fork``使偏移量置$0$，当然测的还是比较弱的，课后发现有人改了 ``fork``使偏移量置 ``0``竟然过了），只是之后如果有读写就会不同。其实处理 ``O_ALONE``还有一种方式就是修改 ``fork.c``中处理写时复制的函数，但是我没试过

fs/fs.h

```c
// ...
int
file_create(char *path, struct File **file);
// ...
```

serv.c

```c
void
serve_open(u_int envid, struct Fsreq_open *rq)
{
	// ...fileid = r;
	if ((r = file_open((char *)path, &f)) < 0 && ((rq->req_omode & O_CREAT) == 0)) {
		ipc_send(envid, r, 0, 0);
		return ;
	}
    file_create(path, &f);
	// o->o_file = f; ...
    // ...ff->f_fd.fd_dev_id = devfile.dev_id;
    if (rq->req_omode & O_ALONE) ipc_send(envid, 0, (u_int)o->o_ff, PTE_V | PTE_R);  // 非共享映射，写时复制
	else ipc_send(envid, 0, (u_int)o->o_ff, PTE_V | PTE_R | PTE_LIBRARY); // 共享
    // 注意fork之前还是共享，不用在fork的过程中将子进程的文件描述符偏移量清空
}
```

lib.h

```c
#define O_APPEND 0x0004
#define O_ALONE 0x0008
```

file.c

```c
int
open(const char *path, int mode)
{
    //...fileid = ffd->f_fileid;
    if (mode & O_APPEND) fd->fd_offset = ffd->f_file.f_size;
    // Step 4: Alloc memory, map the file content into memory. ...
}
```

# Extra

* 注意为什么用 ``fd``接收 ``ans``呢，因为 ``alloc_fd``实际上没有真的分配 ``fd``，它是一个指针，需要通过 ``ficps_open``让服务进程来将其赋值并映射至内存才算分配了，因此它更没有写到 ``tlb ``里面，而如果我们随便找一页接收，那么 ``ipc_send ``实际是直接写的内存，而我们的 ``tlb ``中仍然是旧页，因此无法读取到发来的数据。所以要先 ``syscall_unmap ``一下，刷新 ``tlb ``，下次读取的时候就是直接从内存中读取然后回写 ``tlb``，但是用 ``fd ``的话 ``tlb ``中本来就没有它的地址，所以会直接去内存中读取再回写 ``tlb ``，当然你也可以直接在局部或全局第定义一个数组，但注意一定要强制使其首地址页对齐，同时 ``syscall_unmap``解除一下映射先：
  ```c
  // ...
  char buf[BY2PG<<1]; 
  buf = ROUNDDOWN(buf + BY2PG, BY2PG);
  syscall_unmap(0, buf);
  fsipc(FSREQ_DIR_LIST, req, (u_int)buf, &perm);
  user_bcopy(buf, ans, strlen(buf));
  // ...
  ```
* ``file_open``对于 ``xxx/yyy/zzz``和 ``xxx/yyy/zzz/``都可以打开 ``zzz``的，对于 ``/``它也可以直接打开根目录

lib.h

```c
// ...
int list_dir(const char* path, char* ans);
int fsipc_dir_list(const char *path, char* arr);
// ...
```

file.c

```c
int list_dir(const char *path, char* ans) {
    struct Fd *fd;
    int r;
    if ((r = fd_alloc(&fd)) < 0) return r;
    if((r = fsipc_dir_list(path, (char *) fd)) < 0) return r;
    user_bcopy((const char *)fd, ans, 1024);
    return 0;
}
```

fsipc.c

```c
fsipc_dir_list(const char *path, char *ans)
{
    struct Fsreq_dir_list *req;
    u_int perm;
    req = (struct Fsreq_dir_list *)fsipcbuf;
    strcpy(req->req_path, path);
    return fsipc(FSREQ_DIR_LIST, req, (u_int) ans, &perm);
}
```

include/fs.h

```c
// ...
#define FSREQ_DIR_LIST 8
// ...
struct Fsreq_dir_list {
    u_char req_path[MAXPATHLEN];
};
```

serv.c

```c
char ans[1024];
void
serve_ls(u_int envid, struct Fsreq_dir_list *rq) {
    u_char path[MAXPATHLEN];
    struct File *dir, *f;
    void *blk;
    int r, i, j, nblock, ans_ptr = 0, k;
    user_bcopy(rq->req_path, path, MAXPATHLEN);
    path[MAXPATHLEN - 1] = 0;
    if ((r = file_open((char *)path, &dir)) < 0) {
        ipc_send(envid, r, 0, 0);
        return ;
    }
    nblock = dir->f_size / BY2BLK;
    user_bzero(ans, 1024);
    for (i = 0; i < nblock; i++) {
        r = file_get_block(dir, i, &blk);
        if (r < 0) return r;
        f = (struct File *) blk;
        for (j = 0; j < FILE2BLK; ++j) {
            for (k = 0; k < strlen(f[j].f_name); k++) ans[ans_ptr++] = f[j].f_name[k];
            ans[ans_ptr++] = ' ';
        }
    }
    ans[ans_ptr++] = '\0';
    ipc_send(envid, 0, (u_int) ans, PTE_V | PTE_R);
}
// ...
void
serve(void) {
    // ...
    case FSREQ_DIR_LIST:
    	serve_ls(whom, (struct Fsreq_dir_list *)REQVA);
    	break;
    // ...
}
```

或者不用 ``fsipc``直接一个函数 ``list_dir``，代码量很小：

```c
int list_dir(const char *path, char* ans) {
    int fdnum = open(path, O_RDONLY);
    if (fdnum < 0) {
        return -1;
    }
    struct Fd* fd = num2fd(fdnum);
    struct Filefd *fileFd = (struct Filefd *)fd;
    u_int num = ROUND(fileFd->f_file.f_size, sizeof(struct File)) / sizeof(struct File);
    struct File *f = (struct File*)fd2data(fd);
    int len, i;
    for (i = 0; i < num; i++) {
        if (f[i].f_name[0] != '\0') {
            len = strlen(f[i].f_name);
            user_bcopy(f[i].f_name, ans, len);
            ans += len;
            *ans = ' ';
            ans++;
        }
    }
    *ans = '\0';
    close(fdnum);
    return 0;
}
```

# 总结

* 课上测试终于结束了，可以说 ``os``上机一年比一年难了，基础还行的话 ``exam``倒是基本很快就可以解决，但是 ``extra``有时候真的看命，和水平无关，特别巨的当我没说。虽然过 ``exam``很快，但是我这些总结中有几个 ``extra ``就是课下提交过的（开放课下提交的 ``extra ``好多，基本一个 ``lab ``一次，同时从 ``lab2 ``开始每次课上基本都会延$1$个小时，可以看出难度还是挺大的），所以不要以为$10$来$20$分钟过了 ``exam ``那么 ``extra``就稳了，还是很有可能会在机房坐牢做到$5$点哦
* 评测机有时候也很玄学，有一次同一份代码过半个小时交就过了
* 过课上前几个 ``lab``其实感觉都不在考验你对操作系统的理解，~~有可能是考 ``c``，也有可能是考运气~~，就算理解不到位还是可以过 ``extra ``，所以像 ``lab4-2``这种还是少数
* 刚开始可能会觉得这个操作系统很无聊，但当你做到 ``lab3``，``lab4``左右可能就会发现虽然这些代码很丑，但是这个操作系统变得很有趣了，课上测试的题也变得有趣了
