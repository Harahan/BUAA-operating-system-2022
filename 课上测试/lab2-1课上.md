Exam

不要忘了改 ``page_alloc``

pmap.h

```c
struct Page {
    // ...
    u_short pp_protected;
};
// ...
int page_protect(struct Page *pp);
int page_status_query(struct Page *pp);
```

pmap.c

```c
void page_init(void)
{
	// ...
    // for (; i < size; i++) {
        //pages[i].pp_ref = 1;
		pages[i].pp_protected = 0;
	//}
	// ...
    // for (; i < npage; i++) {
        // pages[i].pp_ref = 0;
		pages[i].pp_protected = 0;
        // LIST_INSERT_HEAD((&page_free_list), (pages + i), pp_link);
    // }
    // ...
}

// ...
int page_protect(struct Page *pp) {
	if (pp->pp_protected == 0) {
		struct Page *page_i;
		LIST_FOREACH(page_i, &page_free_list, pp_link) {
			if (page_i == pp) {
				pp->pp_protected = 1;
				return 0;
			}
		}
		return -1;
	} else return -2;
}

int page_status_query(struct Page *pp) {
	if (pp->pp_protected == 1) return 3;
	struct Page *page_i;
	LIST_FOREACH(page_i, &page_free_list, pp_link) if (page_i == pp) return 2;
	return 1;
}

int page_alloc(struct Page **pp)
{
	struct Page *ppage_temp;
    if (LIST_EMPTY(&page_free_list)) return -E_NO_MEM;
    ppage_temp = LIST_FIRST(&page_free_list);
	int flag = 0;
	LIST_FOREACH(ppage_temp, &page_free_list, pp_link) {
		if (ppage_temp->pp_protected == 0) {
			flag = 1;
			break;
		}
	}
	if (flag) LIST_REMOVE(ppage_temp, pp_link);
	else return -E_NO_MEM;
    // bzero(page2kva(ppage_temp), BY2PG);
    // ...
}
```

# Extra

课上结构体没有 ``alloc``分配空间，直接死亡，课下改了这里，立刻就过了。。。

可以使用全局数组，或者使用 ``alloc``分配相应内存，但是千万不要用没分配内存的野指针，其实可以直接线段树维护的pmap.h中新加声明如下：

```c
typedef struct no{
	int ref;
	u_long vaddr, size;
	struct no *l, *r;
} node;

void buddy_init(void);
int buddy_alloc(u_int size, u_int *pa, u_char *pi);
void buddy_free(u_int pa);
```

pmap.c

首先声明$8$个最大的内存块：

```c
node node_head[8];
```

2个辅助函数 ``init_node``用来初始化伙伴节点，``get_i``用于获取当前伙伴节点管理的内存的$i$（当前管理内存大小$4 × 2^iKB$）

```c
void init_node(int ref, u_long vaddr, u_long size, node *q) {
	q->ref = ref;
	q->vaddr = vaddr; q->size = size;
	q->l = NULL; q->r = NULL;
}

int get_i(u_long t){
	t = t >> 11;
	int ret = 0;
	while(t){t >>= 1; ret += 1;}
	return ret;
}
```

然后是 ``buddy_init``:

```c
void buddy_init(void){
	u_long vaddr = 0x2000000;
	int i = 0;
	for (; i < 8; i++) init_node(0, vaddr + i * (0x2000000>>3), 0x2000000>>3, &(node_head[i]));
}
```

接下来是核心的部分 ``buddy_alloc``以及 ``buddy_free``，我采用均为递归的写法

``buddy_alloc``:

```c
int _buddy_empty(node *head) {
	if (head == NULL) return 1;
	if (head->ref == 1) return 0;
	if (head->size == BY2PG && head->ref == 0) return 1;
	return _buddy_empty(head->l) & _buddy_empty(head->r);
}

int _buddy_alloc(u_int size, u_char *pi, node *head) {
	if (head->ref == 1 || (head->size) < size) return -1;
	if (((head->size) / 2 < size || (head->size) == BY2PG) && _buddy_empty(head)) {
		head->ref = 1;
		*pi = get_i(head->size);
		return head->vaddr;
	}
	node *l, *r;
	if (head->l == NULL) {
		l = (node*)alloc(sizeof(node), 1, 1);
		init_node(0, head->vaddr, (head->size) / 2, l);
		head->l = l;
	}
	if (head->r == NULL) {
		r = (node*)alloc(sizeof(node), 1, 1);
		init_node(0, ((head->vaddr) + (head->size) / 2), (head->size) / 2, r);
		head->r = r;
	}
	int t;
	if ((t=_buddy_alloc(size, pi, head->l)) != -1) return t;
	if ((t=_buddy_alloc(size, pi, head->r)) != -1) return t;
	return -1;
}

int buddy_alloc(u_int size, u_int *pa, u_char *pi) {
	int i = 0, t = -1;
	for (; i < 8; i++) {
		if ((t=_buddy_alloc(size, pi, &node_head[i])) != -1) {
			*pa = (u_int*)(t);
			return 0;
		}
	}
	return -1;
}
```

``buddy_free``

```c
void _buddy_free(u_int pa, node* head) {
	if (pa == head->vaddr && head->ref == 1) {
		head->ref = 0;
		return;
	}
	if ((head->r)->vaddr <= pa) _buddy_free(pa, head->r);
	else _buddy_free(pa, head->l);

}

void buddy_free(u_int pa) {
	int i = 0;
	for (; i < 8; i++) {
		if (pa < node_head[i].vaddr) {
			_buddy_free(pa, &node_head[i - 1]);
			return;
		} else if (pa == node_head[i].vaddr && node_head[i].ref == 1) {
			node_head[i].ref = 0;
			return;
		}
	}
	_buddy_free(pa, &node_head[7]);
}
```

最后放一个很短的数组版的，当然这个是课下写的了：

pmap.c

```c
int base_pa = 0x02000000, a[1<<13], f[1<<13];

void buddy_init(void) {
	int i;
	for (i = 0; i < 8; ++i) a[i<<10] = 10;
}

int buddy_alloc(u_int size, u_int *pa, u_char *pi) {
	int p;
	for (p = 0; p < (1<<13); p += (1<<a[p])) {
		if ((1<<a[p]<<12) < size || f[p]) continue;
		while (a[p] > 0 && (1<<(a[p] - 1)<<12) >= size) {
			a[p] -= 1;
			a[p + (1<<a[p])] = a[p];
		}
		f[p] = 1;
		*pa = (p<<12) + base_pa;
		*pi = a[p];
		return 0;
	}
	return -1;
}

void buddy_free(u_int pa) {
	int k, p = (pa - base_pa)>>12;
	f[p] = 0;
	for (k = a[p]; k < 10; ++k) {
		if (f[p] || f[p^(1<<k)] || a[p^(1<<k)] != a[p]) break;
		p = p&(p^(1<<k));
		f[p] = f[p^(1<<k)] = 0;
		a[p] += 1;
	}
}
```
