/* -*-comment-start: "//";comment-end:""-*-
 * GNU Mes --- Maxwell Equations of Software
 * Copyright © 2026 Gavin John <gavinnjohn@gmail.com>
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

/* The two things lib/windows/ntdll.c cannot say in C, for M2-Planet.
 *
 * Everything else that file needs is ordinary C once it has the PEB, so
 * these two are all that is left here, and lib/windows/x86-mes-mescc/ntlow.c
 * is the same pair written for the other compiler.
 */

#include <windows/ntdll.h>

/* An ntdll routine, by the index lib/m2/x86/ntdll-i386.hex2 put it at.  That
 * table is filled before any C runs, which is why the stages that have no C
 * compiler yet can call ntdll at all; everything compiled from C resolves by
 * name through __ntdll_resolve instead. */
void *
__ntdll (int slot)
{
  asm ("mov____0x8(%ebp),%eax !-4");
  asm ("sal_eax, !2");
  asm ("add_eax, &fn_table");
  asm ("mov_eax,[eax]");
}

/* This process's own PEB, the root of __ntdll_resolve's module walk: the
 * thread information block is at %fs, and the PEB address is the word at
 * 0x30 into it. */
int
__peb (void)
{
  asm ("mov_eax,[fs:DWORD] %0x30");
}
