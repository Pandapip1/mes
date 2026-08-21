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
#ifndef __MES_WINDOWS_X86_SYSCALL_H
#define __MES_WINDOWS_X86_SYSCALL_H 1

/* The counterpart of include/linux/x86/syscall.h, and deliberately empty of
 * numbers.
 *
 * Linux's C library is a table of syscall numbers and a handful of
 * instructions that pass them to the kernel; ../../../lib/windows/ instead
 * looks each routine up in ntdll's export table at startup and calls it, so
 * there is no number to name here and nothing that would want one.  See
 * lib/windows/ntdll.c.
 *
 * The file exists because configure copies include/${kernel}/${cpu}/*.h to
 * include/arch/, and because two headers include <arch/syscall.h> for what it
 * does NOT define: dirent.h asks whether SYS_getdents64 exists to decide how
 * wide a directory entry's inode number is, and here it does not, so a
 * `struct dirent' comes out the 32-bit shape.  Defining a number would change
 * a layout, not enable a call.
 */

#endif /* __MES_WINDOWS_X86_SYSCALL_H */
