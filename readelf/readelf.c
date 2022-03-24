/* This is a simplefied ELF reader.
 * You can contact me if you find any bugs.
 *
 * Luming Wang<wlm199558@126.com>
 */

#include "kerelf.h"
#include <stdio.h>
/* Overview:
 *   Check whether it is a ELF file.
 *
 * Pre-Condition:
 *   binary must longer than 4 byte.
 *
 * Post-Condition:
 *   Return 0 if `binary` isn't an elf. Otherwise
 * return 1.
 */
int is_elf_format(u_char *binary)
{
        Elf32_Ehdr *ehdr = (Elf32_Ehdr *)binary;
        if (ehdr->e_ident[EI_MAG0] == ELFMAG0 &&
                ehdr->e_ident[EI_MAG1] == ELFMAG1 &&
                ehdr->e_ident[EI_MAG2] == ELFMAG2 &&
                ehdr->e_ident[EI_MAG3] == ELFMAG3) {
                return 1;
        }

        return 0;
}

/* Overview:
 *   read an elf format binary file. get ELF's information
 *
 * Pre-Condition:
 *   `binary` can't be NULL and `size` is the size of binary.
 *
 * Post-Condition:
 *   Return 0 if success. Otherwise return < 0.
 *   If success, output address of every section in ELF.
 */

/*
    Exercise 1.2. Please complete func "readelf". 
*/
#define S 40000000000
int readelf(u_char *binary, int size)
{
        Elf32_Ehdr *ehdr = (Elf32_Ehdr *)binary;

        int Nr;

        Elf32_Phdr *phdr1 = NULL; // addr for sh(Elf32_Shdr*)
        Elf32_Phdr *phdr2 = NULL; // addr for sh(Elf32_Shdr*)

        u_char *ptr_ph_table = NULL; // initial addr for sh(u_char*)
        Elf32_Half ph_entry_count;
        Elf32_Half ph_entry_size;


        // check whether `binary` is a ELF file.
        if (size < 4 || !is_elf_format(binary)) {
                printf("not a standard elf format\n");
                return 0;
        }

        // get section table addr, section header number and section header size.
		ph_entry_count = ehdr->e_phnum;
		ph_entry_size = ehdr->e_phentsize;
		ptr_ph_table = binary + ehdr->e_phoff;
		// tips: e_shoff is the offest between the ELF file and section table.
		
        // for each section header, output section number and section addr. 
		/*for (Nr = 0; Nr < sh_entry_count; Nr++) {
			shdr = (Elf32_Shdr*)(ptr_sh_table + Nr * sh_entry_size);
			printf("%d:0x%x\n", Nr, shdr->sh_addr);
		}*/
        // hint: section number starts at 0.
		Elf32_Addr p_paddr1;
		Elf32_Addr p_paddr2;
		Elf32_Addr p_vaddr1;
		Elf32_Addr p_vaddr2;
		Elf32_Word p_filesz1;
		Elf32_Word p_filesz2;
		Elf32_Word p_memsz1;
		Elf32_Word p_memsz2;
		int p1, q1;
		int p2, q2;
		for (int Nr = 0; Nr < ph_entry_count - 1; Nr++){
			phdr1 = (Elf32_Phdr*)(ptr_ph_table + Nr * ph_entry_size);
			phdr2 = (Elf32_Phdr*)(ptr_ph_table + (Nr + 1) * ph_entry_size);
			p_vaddr1 = phdr1->p_vaddr; p_vaddr2 = phdr2->p_vaddr;
			p_filesz1 = phdr1->p_filesz; p_filesz2 = phdr2->p_filesz;
			p1 = p_vaddr1 / S; p2 = p_vaddr2 / S; 
			q1 = (p_vaddr1 + p_filesz1) / S; q2 = (p_vaddr2 + p_filesz2) / S;
			if (q1 <= p2) {
   				printf("%d:0x%x,0x%x\n", Nr, phdr1->p_filesz, phdr1->p_memsz);
			} else if (p_vaddr1 + p_filesz1 <= p_vaddr2 && q1 >= p2) {
				printf("Overlay at page va : 0x%x\n", p_vaddr2);
			} else {
				printf("Conflict at page va : 0x%x\n", p_vaddr2);
			}
		}
   		printf("%d:0x%x,0x%x\n", Nr, phdr1->p_filesz, phdr1->p_memsz);
		
		



        return 0;
}

