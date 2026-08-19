/* GNU Mes --- Maxwell Equations of Software
 * Copyright © 2026 Gavin John
 *
 * This file is part of GNU Mes.
 *
 * GNU Mes is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at
 * your option) any later version.
 *
 * GNU Mes is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Mes.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <windows/x86-mes-m2/ntdll.h>
#include <mes/lib.h>

/* Stands in for lib/linux/brk.c, and unlike the rest of stage0-pe32's Windows
 * layer this one really does ask the operating system for memory.
 *
 * That layer has no brk at all: lib/m2/x86/PE32-i386.hex2 gives the image a
 * section whose VirtualSize runs to the top of 128MB, so everything past the
 * end of the file is already mapped, already writable and already zero, and a
 * program that wants a few megabytes simply uses them.  Every program in that
 * bootstrap fits.
 *
 * Mes does not.  gc_init asks malloc for (ARENA_SIZE + JAM_SIZE) cells at
 * twelve bytes each, which under __M2__ is 264MB in a single call, and there
 * is no image size that answers that honestly.  A Scheme interpreter with a
 * collector wants memory from the system, on demand, which is what brk means.
 *
 * So: reserve a gigabyte of address space once, which costs nothing but
 * address space because MEM_RESERVE commits no pages, and commit the pages as
 * brk walks up it.  That is what brk does on a Unix, and it means Mes pays for
 * the arena it touches rather than the arena it declared.  A megabyte is
 * committed at a time rather than a page, because the alternative is a system
 * call every 4096 bytes of a 264MB allocation.
 *
 * NtAllocateVirtualMemory takes the base and the size by reference and writes
 * back what it actually did -- rounding the base down to a page and the size
 * up -- so the new limit is read back from those rather than assumed.
 */

/* How much address space to reserve, and how much to commit at a time.  The
 * reservation is tried at a gigabyte and halved until it is taken, because how
 * much contiguous space a 32-bit process can get depends on what else is
 * mapped: Windows 11 gives the whole gigabyte, and wine, whose address space
 * is laid out differently, gives 256MB.  Which is why the size is discovered
 * rather than declared -- and why Mes's default arena, at 264MB, does not fit
 * under wine and MES_ARENA has to be set there. */
#define __BRK_RESERVE 0x40000000
#define __BRK_SMALLEST 0x01000000
#define __BRK_CHUNK   0x00100000

#define MEM_COMMIT  0x1000
#define MEM_RESERVE 0x2000
#define PAGE_READWRITE 4

long __brk_ptr;                 /* where brk is now */
long __brk_limit;               /* how far up has been committed */

/* NtAllocateVirtualMemory's two by-reference arguments.  They are globals
 * rather than locals because brk is what malloc calls to get started, so there
 * is nowhere yet to allocate them from. */
long __alloc_addr;
long __alloc_size;

int
__nt_alloc (int type)
{
  int (*NtAllocateVirtualMemory) (int, int, int, int, int, int);
  int *addr;
  int *size;

  NtAllocateVirtualMemory = __ntdll (NT_ALLOC);
  addr = &__alloc_addr;         /* not in the argument list: see ntdll.c */
  size = &__alloc_size;

  /* forwards: NtAllocateVirtualMemory (-1, addr, 0, size, type,
   *                                    PAGE_READWRITE) -- -1 is the
   *                                    pseudo-handle for this process */
  return NtAllocateVirtualMemory (PAGE_READWRITE, type, size, 0, addr, -1);
}

long
brk (void *addr)
{
  long want;
  long need;

  if (__brk_ptr == 0)
    {
      long try;
      int rc;

      try = __BRK_RESERVE;
      rc = -1;
      while (try >= __BRK_SMALLEST)
        {
          __alloc_addr = 0;     /* 0 lets the system choose where */
          __alloc_size = try;
          rc = __nt_alloc (MEM_RESERVE);
          if (rc == 0)
            break;
          try = try / 2;
        }
      if (rc != 0)
        return -1;
      __brk_ptr = __alloc_addr;
      __brk_limit = __alloc_addr;
    }

  if (addr == 0)
    return __brk_ptr;

  want = addr;
  if (want > __brk_limit)
    {
      need = want - __brk_limit;
      need = need + __BRK_CHUNK;
      __alloc_addr = __brk_limit;
      __alloc_size = need;
      if (__nt_alloc (MEM_COMMIT) != 0)
        return -1;
      /* What it actually committed, which is at or below what was asked for
       * at the bottom and at or above it at the top. */
      __brk_limit = __alloc_addr + __alloc_size;
    }

  __brk_ptr = want;
  return __brk_ptr;
}
