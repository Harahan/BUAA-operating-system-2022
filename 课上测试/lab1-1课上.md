# Exam

readelf.c

```c
int readelf(u_char *binary, int size)
{
	Elf32_Ehdr *ehdr = (Elf32_Ehdr *)binary;
    int Nr;
    Elf32_Shdr *shdr = NULL; // addr for sh(Elf32_Shdr*)
    u_char *ptr_sh_table = NULL; // initial addr for sh(u_char*)
    Elf32_Half sh_entry_count;
    Elf32_Half sh_entry_size;
    if (size < 4 || !is_elf_format(binary)) {
        printf("not a standard elf format\n");
        return 0;
    }
	sh_entry_count = ehdr->e_shnum;
    sh_entry_size = ehdr->e_shentsize;
    ptr_sh_table = binary + ehdr->e_shoff;
	Elf32_Shdr *shdr1 =  (Elf32_Shdr*)(ptr_sh_table + 2 * sh_entry_size);
	Elf32_Shdr *shdr2 =  (Elf32_Shdr*)(ptr_sh_table + 3 * sh_entry_size);
	printf("Read : %d:0x%x,0x%x\n", 2, shdr1->sh_offset, shdr1->sh_addr);
	printf("Read : %d:0x%x,0x%x\n", 3, shdr2->sh_offset, shdr2->sh_addr);
	return 0;
}
```

# Extra

其实不用二重循环去寻找有没有页冲突，注意每个 ``phdr->p_vaddr``其实是按顺序排的，相关内容在课程组给的那个啥关于 ``elf``的阅读资料中

readelf.c

```c
int down(int m) {
	return m - m % BY2PG; 
}

int readelf(u_char *binary, int size)
{
    Elf32_Ehdr *ehdr = (Elf32_Ehdr *)binary;
	Elf32_Phdr *phdr1 = NULL;
	Elf32_Phdr *phdr2 = NULL;
	u_char *ptr_ph_table = NULL;
	Elf32_Half ph_entry_count;
	Elf32_Half ph_entry_size;
	int Nr;
    if (size < 4 || !is_elf_format(binary)) {
       	printf("not a standard elf format\n");
        return 0;
    }
    ptr_ph_table = binary + ehdr->e_phoff;
    ph_entry_size = ehdr->e_phentsize;
    ph_entry_count = ehdr->e_phnum;
	int ans_addr, flag = 0;
	for(int i = 0; i < ph_entry_count; i++){
        phdr1 = (Elf32_Phdr *)(ptr_ph_table + i * ph_entry_size);
        int l1 = phdr1->p_vaddr, r1 = l1 + phdr1->p_memsz;
        for(int j = i + 1; j < ph_entry_count; j++) {
            phdr2 = (Elf32_Phdr *)(ptr_ph_table + j * ph_entry_size);
            int l2 = phdr2->p_vaddr, r2 = l2 + phdr2->p_memsz;
            if(r1 < r2 && down(l2) == down(r1)) {
                ans_addr = down(l2);
                if(l2 > r1) {flag = 1; break;} else {flag = 2; break;}
            } else if (r2 < r1 && down(r2) == down(l1)) {
                ans_addr = down(r2);
                if(l1 > r2) {flag = 1; break;} else {flag = 2; break;}
            } 
        }
        if(flag != 0) break;
   	}
    if (flag == 0) {
        for(Nr = 0;Nr < ph_entry_count;Nr++) {
            phdr1 = (Elf32_Phdr *)(ptr_ph_table + Nr * ph_entry_size);
            printf("%d:0x%x,0x%x\n",Nr,phdr1->p_filesz,phdr1->p_memsz);
        }
    }else if (flag == 1) printf("Overlay at page va : 0x%x\n",ans_addr);
    else printf("Conflict at page va : 0x%x\n",ans_addr);
    return 0;
}
```
