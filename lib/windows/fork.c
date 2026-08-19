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

/* Stands in for lib/linux/fork.c, and fails.
 *
 * fork's whole meaning is that the child comes back from the same call with
 * the same memory and carries on from the same line, and Windows has no call
 * that does that.  ntdll exports RtlCloneUserProcess, which is the nearest
 * thing; it clones an address space the Win32 side of the system knows
 * nothing about, and wine does not export it at all, so it could not be
 * tested here even if it were the right answer.
 *
 * What Windows can do instead is start a program directly, which is
 * lib/windows/execve.c, and wait for it, which is lib/windows/waitpid.c.  A
 * caller wanting a child says
 *
 *   pid = __spawn (file, argv, envp); waitpid (pid, &status, 0);
 *
 * rather than fork, execve in the child and waitpid in the parent.
 */

int
fork ()
{
  return -1;
}
