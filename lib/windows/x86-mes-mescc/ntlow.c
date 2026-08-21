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

/* The two things lib/windows/ntdll.c cannot say in C, for MesCC.
 *
 * Everything else that file needs -- walking the module list, reading the
 * export table, finding a routine by name -- is ordinary C once it has the
 * PEB, so only the PEB read is here, and it is one instruction.
 *
 * The M2 port has a third, __ntdll (slot), which reads the table
 * lib/m2/x86/ntdll-i386.hex2 fills before any C runs.  There is no such
 * table here: MesCC compiles no stage that predates C, and everything
 * resolves by name through __ntdll_resolve.  __ntdll is defined all the
 * same, returning nothing, because <windows/ntdll.h> declares it and a
 * caller that reached it would be a bug worth crashing on rather than a
 * link error in a build that has no linker yet.
 */

#include <windows/ntdll.h>

/* This process's own PEB.  The thread information block is at %fs, and the
 * PEB address is the word at 0x30 into it. */
int
__peb (void)
{
  asm ("mov____%fs:0x32,%eax %0x30");
}

/* No slot table exists in a MesCC build; see the note above. */
void *
__ntdll (int slot)
{
  return 0;
}
