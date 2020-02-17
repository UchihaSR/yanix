/**
 * kernel/execute/elf.c
 * 
 * Author: Robbe De Greef
 * Date:   24 may 2019
 * 
 * Version 1.0
 */

#include <yanix/elf.h>
#include <mm/paging.h>
#include <proc/tasking.h>
#include <libk/string.h>
#include <stdint.h>
#include <stddef.h>
#include <errno.h>
#include <kernel.h>


#define ELF_PHDR_NULL		0
#define ELF_PHDR_LOAD		1
#define ELF_PHDR_DYNAMIC	2
#define ELF_PHDR_INTERP		3
#define ELF_PHDR_NOTE		4
#define ELF_PHDR_SHLIB		5
#define ELF_PHDR_PHDR		6
#define ELF_PHDR_LOPROC 	0x70000000
#define ELF_PHDR_HIPROC		0x7fffffff

#define ELF_SHDR_WRITE 		0x1
#define ELF_SHDR_ALLOC 		0x2
#define ELF_SHDR_EXECINSTR 	0x4
#define ELF_SHDR_MASKPROC 	0xf0000000

#define ELF_R 	4
#define ELF_W	2
#define ELF_X 	1


#define ELF_SHDR_TYPE_NULL		0
#define ELF_SHDR_TYPE_PROGBITS	1
#define ELF_SHDR_TYPE_SYMTAB	2
#define ELF_SHDR_TYPE_STRTAB	3
#define ELF_SHDR_TYPE_RELA		4
#define ELF_SHDR_TYPE_HASH		5
#define ELF_SHDR_TYPE_DYNAMIC	6
#define ELF_SHDR_TYPE_NOTE		7
#define ELF_SHDR_TYPE_NOBITS	8
#define ELF_SHDR_TYPE_REL 		9
#define ELF_SHDR_TYPE_SHLIB		10
#define ELF_SHDR_TYPE_DYNSYM	11

#define ELF_SHDR_TYPE_LOPROC	0x70000000
#define ELF_SHDR_TYPE_HIPROC	0x7fffffff
#define ELF_SHDR_TYPE_LOUSER	0x80000000
#define ELF_SHDR_TYPE_HIUSER	0xffffffff


/**
 * @brief      Checks elf file support
 *
 * @param      hdr   The header
 *
 * @return     { description_of_the_return_value }
 */
static int elf_check_support(elf32_hdr_t *hdr)
{
	if (hdr->magic != ELF_MAGIC) {
		// not an elf file
		// exec format error
		errno = ENOEXEC;
		return -1;
	} else if (hdr->bit_type != ELF_BIT32) {
		// 32 bit not supported
		// exec format error
		errno = ENOEXEC;
		return -1;
	} else if (hdr->arch_type != ELF_ARCH_X86) {
		// not 32 bit target
		// exec format error
		errno = ENOEXEC;
		return -1;
	} else if (hdr->elf_header_version < ELF_SUPPORTED) {
		// elf header not supported
		// exec format error
		errno = ENOEXEC;
		return -1;
	//} else if (hdr->type != ELF_TYPE_REL && hdr->type != ELF_TYPE_EXEC){
	} else if (hdr->type != ELF_TYPE_EXEC) {
		// elf file type not supported
		// exec format error
		errno = ENOEXEC;
		return -1;
	}
	return 0;
}

#include <debug.h>

/**
 * @brief      Loads a program header into memory
 *
 * @param      file  The file pointer
 * @param      phdr  The program header
 *
 * @return     success
 */
static int _elf_load_pheader(void *file, elf32_phdr_t *phdr)
{
	map_mem(phdr->vaddr, phdr->vaddr + phdr->memsize, 0, phdr->flags & ELF_W);
	memset((void*) phdr->vaddr, 0, phdr->memsize);
	memcpy((void*) phdr->vaddr, (void*) (((uint32_t) file) + phdr->offset), phdr->filesize);
	if (phdr->filesize < phdr->memsize) {
		memset((void*)(phdr->vaddr + phdr->filesize), 0, phdr->memsize-phdr->filesize);
	}
	return  0; 
}

/**
 * @brief      Loops over the program header
 *
 * @param      file    The file pointer
 * @param      phdr    The program header
 * @param[in]  size    The size of a program header entry
 * @param[in]  amount  The amount of program header entries
 *
 * @return     success
 */
static int _elf_loop_over_program_table(void *file, elf32_hdr_t* elf_hdr, elf32_phdr_t* elf_program_table)
{
	for (size_t i = 0; i < elf_hdr->pheader_table_amount; i++) {
		if (elf_program_table[i].filesize > elf_program_table[i].memsize) {
			errno = ENOEXEC;
			return -1;
		
		} else if (elf_program_table[i].type == ELF_PHDR_NULL) { 
			continue;

		} else if (elf_program_table[i].type == ELF_PHDR_LOAD) {
			if (_elf_load_pheader(file, &elf_program_table[i]) == -1) {
				return -1;
			}
		}

	}

	/* Find the program break */

	unsigned long pbrk = elf_program_table[0].vaddr + elf_program_table[0].memsize;

	for (size_t i = 1; i < elf_hdr->pheader_table_amount; i++)
	{
		if ((elf_program_table[i].vaddr + elf_program_table[i].memsize) > pbrk)
			pbrk = elf_program_table[i].vaddr + elf_program_table[i].memsize;
	}
	
	get_current_task()->program_break = pbrk;
	
	return 0;
}

static int _elf_loop_over_section_table(elf32_hdr_t* elf_hdr, elf32_shdr_t* elf_section_table)
{
	(void) (elf_section_table);

	for (size_t i = 0; i < elf_hdr->sheader_table_amount; i++) {
		// we gotta loop over all the sections and make a section to segment mapping, then map the segments into the tasks struct
		// so that we can modify the program break and create a sbrk function for the malloc of newlib
		// the mapping probably can be calculated with the vaddresses and the section sizes (sectionhdr->size)
		
		// oke scrap all this bs im just gonna look for the most outward address and put that as a program break
		// (program headers)
		
	}
	return 0;
}

uint32_t load_elf_into_mem(void* file) 
{
	// check if it is executable
	elf32_hdr_t *elf_hdr = (elf32_hdr_t*) file;
	if (elf_check_support(elf_hdr) == -1) {
		return 0;
	}

	// locating the header tables
	elf32_shdr_t *elf_section_table = (elf32_shdr_t*) ((uint32_t)file + elf_hdr->sheader_table_position);
	elf32_phdr_t *elf_program_table = (elf32_phdr_t*) ((uint32_t)file + elf_hdr->pheader_table_position);
	
	// looping over the header tables
	if (_elf_loop_over_section_table(elf_hdr, elf_section_table) == -1) {
		return 0;
	}


	if (_elf_loop_over_program_table(file, elf_hdr, elf_program_table) == -1) {
		return 0;
	}

	return elf_hdr->entry;
}