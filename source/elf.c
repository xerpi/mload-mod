/*
	Custom IOS Module (MLOAD)

	Copyright (C) 2008 neimod.
	Copyright (C) 2010 Hermes.
	Copyright (C) 2010 Waninkoko.

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include <stdio.h>
#include <string.h>

#include <elf.h>
#include "elf.h"
#include "swi_mload.h"
#include "syscalls.h"
#include "types.h"

/* Variables */
elfdata elf = { 0 };

static inline u32 be32(u8 *val)
{
	return (val[0] << 24) | (val[1] << 16) | (val[2] << 8) | val[3];
}

s32 Elf_Load(void *data)
{
	Elf32_Ehdr *head = (Elf32_Ehdr *)data;

	u32 i, j, pos;

	if ((head->e_ident[EI_MAG0] != ELFMAG0) ||
	    (head->e_ident[EI_MAG1] != ELFMAG1) ||
	    (head->e_ident[EI_MAG2] != ELFMAG2) ||
	    (head->e_ident[EI_MAG3] != ELFMAG3)) {
		return -1;
	}

	/* Initial position */
	pos = head->e_phoff;

	/* Entry point */
	elf.start = (void *)head->e_entry;

	/* Load sections */
	for (i = 0; i < head->e_phnum; i++) {
		Elf32_Phdr *entry = (data + pos);

		if (entry->p_type == 4) {
			u8 *addr = (data + entry->p_offset);

			if (be32(addr))
				return -2;

			for (j = 4; j < entry->p_memsz; j += 8) {
				u32 val1 = be32(&addr[j]);
				u32 val2 = be32(&addr[j+4]);

				switch (val1) {
				case 0x9:
					elf.start = (void *)val2;
					break;
				case 0xB:
					elf.arg = (void *)val2;
					break;
				case 0x7D:
					elf.prio  = val2;
					break;
				case 0x7E:
					elf.size_stack = val2;
					break;
				case 0x7F:
					elf.stack = (void *)val2;
					break;
				}
			}
		}

		if (entry->p_type == 1 && entry->p_memsz && entry->p_vaddr) {
			void *dst = (void *)entry->p_vaddr;
			void *src = (data + entry->p_offset);

			/* Invalidate cache */
			os_sync_before_read(dst, entry->p_memsz);

			/* Copy section */
			memset(dst, 0,   entry->p_memsz);
			memcpy(dst, src, entry->p_filesz);

			/* Flush cache */
			os_sync_after_write(dst, entry->p_memsz);
		}

		/* Move position */
		pos += sizeof(Elf32_Phdr);
	}

	return 0;
}

s32 Elf_LoadFromSD(const char *path, elfdata *elf)
{
	int fd, ret;
	u32 tmp;
	Elf32_Ehdr head;
	Elf32_Phdr phdr;

	fd = os_open(path, IOS_OPEN_READ);
	if (fd < 0)
		return fd;

	ret = os_read(fd, &head, sizeof(head));
	if (ret < 0)
		goto err_close;

	if ((head.e_ident[EI_MAG0] != ELFMAG0) ||
	    (head.e_ident[EI_MAG1] != ELFMAG1) ||
	    (head.e_ident[EI_MAG2] != ELFMAG2) ||
	    (head.e_ident[EI_MAG3] != ELFMAG3)) {
		ret = -1;
		goto err_close;
	}

	elf->start = (void *)head.e_entry;

	for (Elf32_Half i = 0; i < head.e_phnum; i++) {
		ret = os_seek(fd, head.e_phoff + i * sizeof(phdr), SEEK_SET);
		if (ret < 0)
			goto err_close;

		ret = os_read(fd, &phdr, sizeof(phdr));
		if (ret < 0)
			goto err_close;

		//printf("Segment: type: %ld, p_vaddr: 0x%lx, p_memsz: 0x%04lx, p_filesz: 0x%04lx\n",
		//	phdr.p_type, phdr.p_vaddr, phdr.p_memsz, phdr.p_filesz);

		if (phdr.p_type == PT_NOTE) {
			ret = os_seek(fd, phdr.p_offset, SEEK_SET);
			if (ret < 0)
				goto err_close;

			ret = os_read(fd, &tmp, sizeof(tmp));
			if (ret < 0)
				goto err_close;

			if (tmp != 0) { // bad info (sure)
				ret = -2;
				goto err_close;
			}

			for (u32 m = 4; m < phdr.p_memsz; m += 8) {
				ret = os_seek(fd, phdr.p_offset + m, SEEK_SET);
				if (ret < 0)
					goto err_close;

				ret = os_read(fd, &tmp, sizeof(tmp));
				if (ret < 0)
					goto err_close;

				switch (tmp) {
				case 0x9:
					ret = os_read(fd, &elf->start, sizeof(elf->start));
					if (ret < 0)
						goto err_close;
					break;
				case 0xB:
					ret = os_read(fd, &elf->arg, sizeof(elf->arg));
					if (ret < 0)
						goto err_close;
					break;
				case 0x7D:
					ret = os_read(fd, &elf->prio, sizeof(elf->prio));
					if (ret < 0)
						goto err_close;
					break;
				case 0x7E:
					ret = os_read(fd, &elf->size_stack, sizeof(elf->size_stack));
					if (ret < 0)
						goto err_close;
					break;
				case 0x7F:
					ret = os_read(fd, &elf->stack, sizeof(elf->stack));
					if (ret < 0)
						goto err_close;
					break;
				}
			}
		} else if (phdr.p_type == PT_LOAD && phdr.p_memsz != 0 && phdr.p_vaddr != 0) {
			os_sync_before_read((void *)phdr.p_vaddr, phdr.p_memsz);

			memset((void *)phdr.p_vaddr, 0, phdr.p_memsz);

			ret = os_seek(fd, phdr.p_offset, SEEK_SET);
			if (ret < 0)
				goto err_close;

			ret = os_read(fd, (void *)phdr.p_vaddr, phdr.p_filesz);
			if (ret < 0)
				goto err_close;

			os_sync_after_write((void *)phdr.p_vaddr, phdr.p_memsz);
			//DCFlushRange((void *)phdr.p_vaddr, phdr.p_memsz);
		}
	}

	//ICInvalidate();

	ret = 0;
err_close:
	os_close(fd);

	return ret;
}

s32 Elf_Run(void)
{
	/* Create thread */
	return Elf_RunThread(elf.start, elf.arg, elf.stack, elf.size_stack, elf.prio);
}

s32 Elf_RunThread(void *start, void *arg, void *stack, u32 stacksize, u32 priority)
{
	s32 ret;

	/* Create thread */
	ret = os_thread_create(start, arg, stack, stacksize, priority, 0);
	if (ret >= 0)
		os_thread_continue(ret);

	return ret;
}

s32 Elf_StopThread(s32 tid)
{
	/* Stop thread */
	return os_thread_stop(tid);
}

s32 Elf_ContinueThread(s32 tid)
{
	/* Continue thread */
	return os_thread_continue(tid);
}
