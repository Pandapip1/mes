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
 * Windows does have a fork primitive: NtCreateProcessEx given a parent and no
 * section handle clones the parent's address space instead of mapping an
 * image -- ReactOS's own PspCreateProcess reaches that branch and says "This
 * is a clone!" before declining to implement it -- and RtlCloneUserProcess
 * wraps it and is meant to return in both processes.
 *
 * It does not work.  On Windows 11 22621 the parent gets STATUS_SUCCESS and a
 * real cloned process, and the child's thread, which is not suspended, sits in
 * Wait and never reaches the first statement after the call.  Every flag
 * combination is the same, and the identical call from 64-bit and 32-bit
 * PowerShell behaves the same way, so it is neither this port nor WOW64.  The
 * measurements are written up beside the code in stage0-pe32's
 * x86/M2libc-windows/process.c, where __clone_process keeps the call.
 *
 * Failing here is deliberate: a fork whose parent gets a handle and whose
 * child never runs would hang the first caller to wait for it.  What Windows
 * can do is start a program directly, which is lib/windows/execve.c, and wait
 * for it, which is lib/windows/waitpid.c.  A caller wanting a child says
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
