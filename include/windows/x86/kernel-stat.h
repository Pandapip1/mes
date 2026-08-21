/* -*-comment-start: "//";comment-end:""-*-
 * GNU Mes --- Maxwell Equations of Software
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
#ifndef __MES_WINDOWS_X86_KERNEL_STAT_H
#define __MES_WINDOWS_X86_KERNEL_STAT_H 1

/* What <sys/stat.h> means by `struct stat'.
 *
 * On Linux this is not a choice: the kernel writes these bytes and the header
 * has to describe them field for field.  Windows writes nothing of the kind
 * -- NtQueryInformationFile fills in structures of its own -- so what a stat
 * looks like here is this port's decision, and the decision is to look like
 * the Linux one.  Nothing above the C library then has to know which kernel
 * it was compiled for, and a stat implemented against ntdll has somewhere
 * obvious to put each thing it learns.
 *
 * Which has not happened yet: lib/stub/stat.c is what answers a stat today.
 * The struct is here for the code that names its fields, not for a caller
 * that will find them filled in.
 *
 * The layout is Linux's 32-bit one, from
 * arch/x86/include/uapi/asm/stat.h; the 64-bit variant beside it there is
 * about a syscall ABI this has no part in.
 */

/* *INDENT-OFF* */
struct stat
{
  unsigned long  st_dev;
  unsigned long  st_ino;
  unsigned short st_mode;
  unsigned short st_nlink;
  unsigned short st_uid;
  unsigned short st_gid;
  unsigned long  st_rdev;
  unsigned long  st_size;
  unsigned long  st_blksize;
  unsigned long  st_blocks;
  unsigned long  st_atime;
  unsigned long  st_atime_usec;
  unsigned long  st_mtime;
  unsigned long  st_mtime_usec;
  unsigned long  st_ctime;
  unsigned long  st_ctime_usec;
  unsigned long  __pad0;
  unsigned long  __pad1;
};
/* *INDENT-ON* */

#endif /* __MES_WINDOWS_X86_KERNEL_STAT_H */
