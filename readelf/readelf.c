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
int readelf(u_char *binary, int size)
{
        Elf32_Ehdr *ehdr = (Elf32_Ehdr *)binary;

        int Nr;

        Elf32_Shdr *shdr = NULL; // addr for sh(Elf32_Shdr*)

        u_char *ptr_sh_table = NULL; // initial addr for sh(u_char*)
        Elf32_Half sh_entry_count;
        Elf32_Half sh_entry_size;


        // check whether `binary` is a ELF file.
        if (size < 4 || !is_elf_format(binary)) {
                printf("not a standard elf format\n");
                return 0;
        }

        // get section table addr, section header number and section header size.
		sh_entry_count = ehdr->e_shnum;
		sh_entry_size = ehdr->e_shentsize;
		ptr_sh_table = binary + ehdr->e_shoff;
		// tips: e_shoff is the offest between the ELF file and section table.
		
        // for each section header, output section number and section addr. 
		/*for (Nr = 0; Nr < sh_entry_count; Nr++) {
			shdr = (Elf32_Shdr*)(ptr_sh_table + Nr * sh_entry_size);
		sh_entry_count = ehdr->e_shnum;
		sh_entry_count = ehdr->e_shnum;
		sh_entry_size = ehdr->e_shentsize;
		ptr_sh_table = binary + ehdr->e_shoff;
		sh_entry_size = ehdr->e_shentsize;
		ptr_sh_table = binary + ehdr->e_shoff;
			printf("%d:0x%x\n", Nr, shdr->sh_addr);
		}*/
	Elf32_Shdr shdr1 =  (Elf32_Shdr*)(ptr_sh_table + 2 * sh_entry_size);

	Elf32_Shdr shdr2 =  (Elf32_Shdr*)(ptr_sh_table + 3 * sh_entry_size);

	printf("Read : %d:0x%x,0x%x\n", 2, shdr1->sh_offset, shdr1->sh_addr);

	printf("Read : %d:0x%x,0x%x\n", 3, shdr2->sh_offset, shdr2->sh_addr);
	
        // hint: section number starts at 0.
		
        return 0;
}

