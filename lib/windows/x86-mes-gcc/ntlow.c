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

/* The one thing lib/windows/ntdll.c cannot say in C, for a real C compiler.
 *
 * The MesCC file this stands beside has to say the same thing in that
 * compiler's own assembly dialect (lib/windows/x86-mes-mescc/ntlow.c); here
 * it is ordinary GNU inline assembly, which is what TinyCC reads.
 *
 * Everything else that file needs -- walking the module list, reading the
 * export table, finding a routine by name -- is ordinary C once it has the
 * PEB, so only the PEB read is here, and it is one instruction.
 *
 * __ntdll (slot) reads the table lib/m2/x86/ntdll-i386.hex2 fills before any
 * C runs.  There is no such table by the time a compiler this good is doing
 * the building: every stage from here up resolves by name through
 * __ntdll_resolve.  It is defined all the same, returning nothing, because
 * <windows/ntdll.h> declares it and a caller that reached it would be a bug
 * worth crashing on rather than a link error.
 */

#include <windows/ntdll.h>

/* This process's own PEB.  The thread information block is at %fs, and the
 * PEB address is the word at 0x30 into it. */
int
__peb (void)
{
  int peb;

  __asm__ ("movl %%fs:0x30, %0" : "=r" (peb));
  return peb;
}

/* No slot table exists in a build this late; see the note above. */
void *
__ntdll (int slot)
{
  return 0;
}
