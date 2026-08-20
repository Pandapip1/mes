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

/* Stands in for lib/linux/fork.c, and fails -- for now, and not for the
 * reasons an earlier version of this file gave.
 *
 * Windows does have a fork primitive and it does not work.  The measurements
 * live beside the code in stage0-pe32's x86/M2libc-windows/process.c, where
 * __clone_process keeps the call and does not make it.  Three things this
 * file used to say about them have since turned out to be wrong, and are
 * worth correcting rather than leaving:
 *
 *   RtlCloneUserProcess does not wrap NtCreateProcessEx.  Disassembling
 *   ntdll32.dll shows it reaching ZwCreateUserProcess, the same system call
 *   behind ordinary process creation.  They are two unrelated ways into the
 *   kernel, so a defect found in one says nothing about the other.
 *
 *   The flag combinations are not all alike.  Without NO_SYNCHRONIZE the
 *   child deadlocks; with it the child runs and dies of an access violation.
 *   And the deadlock turned out to be downstream of the fault rather than a
 *   second failure beside it: what waits on the inherited lock is the
 *   exception dispatcher, trying to report the access violation.
 *
 *   It is not "neither this port nor WOW64".  That was drawn from the same
 *   call failing from PowerShell too, where the child was stopped by the
 *   inherited lock -- a different failure.  Where the clone's child actually
 *   stops is `mov eax, fs:0x18', faulting on linear address 0x18: the FS
 *   selector is the right one and the descriptor behind it has no base.
 *
 * stage0-pe32 has a working fork now, and uses none of that.  It starts the
 * same program again and copies the parent over the child -- the way Cygwin
 * has always done it -- which works there because its image is at a fixed
 * address, with code, globals, heap and even the kernel's own stack laid out
 * identically in every process that runs it.  A pointer means the same thing
 * on both sides, so nothing needs fixing up.
 *
 * Mes does not get that for free, and lib/windows/brk.c is why.  That layer
 * keeps its heap inside its image; this one asks the operating system for up
 * to a gigabyte and lives there, at a base the kernel chooses and a size
 * discovered by halving until a reservation is taken.  So a fork here would
 * have to put the child's arena at the parent's base rather than wherever the
 * child's own startup landed it, and then copy as much of it as is committed
 * -- which for an interpreter that has already grown its arena is most of the
 * cost of the fork.  Worth doing when something needs it; not the same
 * afternoon's work the other one was.
 *
 * Until then: what Windows can do is start a program directly, which is
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
