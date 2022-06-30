# Exam

* 注意按页表查找本来就是按顺序查找，所以不用再排序
* 不用考虑查找的是页表或者页目录，不然就要排序了

pmap.h

```c
int inverted_page_lookup(Pde *pgdir, struct Page *pp, int vpn_buffer[]);
```

pmap.c

```c
int inverted_page_lookup(Pde *pgdir, struct Page *pp, int vpn_buffer[]) {
	int i, cnt = 0, j;
	for (i = 0; i < 1024; ++i) {
		Pde *pgdir_entryp = pgdir + i;
		if ((*pgdir_entryp) & PTE_V) {
			Pte *pgtable = KADDR(PTE_ADDR(*pgdir_entryp));
			for (j = 0; j < 1024; ++j) {
				Pte *pgtable_entryp = pgtable + j;
				if ((*pgtable_entryp) & PTE_V & page2pa(pp) == PTE_ADDR(*pgtable_entryp)) 
                    	vpn_buffer[cnt++] = (i <<10) + j;
				}
			}
		}
	}
	return cnt;
}
```

# Extra

* 注意 ``bcopy``用虚拟地址
* 注意清空 ``pp_ref``，这样才可以真正 ``remove``掉该页，以及 ``pp_ref``的赋值

pmap.h

```c
struct Page* page_migrate(Pde *pgdir, struct Page *pp);
```

pmap.c

```c
struct Page_list page_free_list;	/* Free list of physical pages */
struct Page_list fast_page_free_list;
// ...
void page_init(void)
{
	// ...
	LIST_INIT(&fast_page_free_list);
	// ...
    for (; i < 12288; i++) {
        // ...
    }
    for (; i < npage; i++) {
        pages[i].pp_ref = 0;
        LIST_INSERT_HEAD((&fast_page_free_list), (pages + i), pp_link);
	}
}

void page_free(struct Page *pp)
{
	// ...
    if (pp->pp_ref == 0 && (page2pa(pp) >> 20) < 48) {
        LIST_INSERT_HEAD((&page_free_list), pp, pp_link);
        return;
    } else if (pp->pp_ref == 0) {
        LIST_INSERT_HEAD((&fast_page_free_list), pp, pp_link);
        return;
	}
	// ...
}

struct Page* page_migrate(Pde* pgdir, struct Page *pp) {
	struct Page *tp;
	if ((page2pa(pp)>>20) < 48) tp = LIST_FIRST(&fast_page_free_list);
	else tp = LIST_FIRST(&page_free_list);
	LIST_REMOVE(tp, pp_link);
	bcopy(page2kva(pp), page2kva(tp), BY2PG);
	int i, cnt = 0, j;
	for (i = 0; i < 1024; i++) {
		Pde *pgdir_entryp = pgdir + i;
		if ((*pgdir_entryp) & PTE_V) {
			Pte *pgtable = KADDR(PTE_ADDR(*pgdir_entryp));
			for (j = 0; j < 1024; j++) {
				Pte *pgtable_entryp = pgtable + j;
				if ((*pgtable_entryp) & PTE_V & PTE_ADDR(*pgtable_entryp) == page2pa(pp)) {
					cnt++;
					*pgtable_entryp = (*pgtable_entryp) & 0xFFF;
					*pgtable_entryp |= page2pa(tp);
				}
			}
		}
	}
	tp->pp_ref = cnt;
	pp->pp_ref = 0;
	page_free(pp);
	return tp;
}
```
