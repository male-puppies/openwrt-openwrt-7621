/* hfload.c
 *
 * This file is subject to the terms and conditions of the GNU
 * General Public License.  See the file "COPYING" in the main
 * directory of this archive for more details.
 *
 * Copyright (C) 2000, Jay Carlson
 */

/*
 * Boot loader main program.
 */
#include <linux/kconfig.h>

// for linux-3.10
#include <asm-generic/unistd.h>
//#include <linux/elf.h>
#include "hfload.h"

#define REG32(reg)   (*(volatile unsigned int   *)((unsigned int)reg))

#if defined(CONFIG_RTL_819X)
	#define BASE_ADDR 0xB8000000
	#define set_io_port_base(base)	\
		do { * (unsigned long *) &mips_io_port_base = (base); } while (0)	
	
	const unsigned long mips_io_port_base;
#endif

int file_offset;
unsigned long kernelStartAddr;
/*
int old_stack_pointer;

#define MAX_PHDRS_SIZE 8

Elf32_Ehdr header;
Elf32_Phdr phdrs[MAX_PHDRS_SIZE];
*/
extern void flush_cache(void);

/*
void
zero_region(char *start, char *end)
{
	char *addr;
	int count;

	count = end - start;
#ifndef __DO_QUIET__
	printf("zeroing from %08x to to %08x, 0x%x bytes\n", start, end, count);
#endif

#ifndef FAKE_COPYING
	memset(start, 0, count);
#endif
}

void
load_phdr(Elf32_Phdr *phdr)
{
	char *addr, *end;
	
	seek_forward(phdr->p_offset);
	
	addr = (char *)phdr->p_vaddr;
	end = ((char *)addr) + phdr->p_memsz;
	
	copy_to_region(addr, phdr->p_filesz);
	
	addr = ((char *)addr) + phdr->p_filesz;
	
	zero_region(addr, end);
}
*/

int main(unsigned long stack_start_addr)
{
	int i;
	//Elf32_Ehdr *pHdr;
	
	file_offset = 0;

 	set_io_port_base(BASE_ADDR);

	printf("decompressing kernel:\n");
	decompress_kernel(UNCOMPRESS_OUT, stack_start_addr+4096, FREEMEM_END, 0);
	printf("done decompressing kernel.\n");

	flush_cache();
	
	printf("start address: 0x%08x\n", kernelStartAddr);
	start_kernel(kernelStartAddr);
	//start_kernel(0x80000000);
}
