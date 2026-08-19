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

/* Stands in for lib/linux/x86-mes-m2/_exit.c, which is three instructions
 * around int $0x80.  NtTerminateProcess takes the process to end and the
 * status to end it with; -1 is the pseudo-handle meaning this one. */

void
_exit (int code)
{
  int (*NtTerminateProcess) (int, int);

  NtTerminateProcess = __ntdll (NT_EXIT);
  /* forwards: NtTerminateProcess (-1, code) */
  NtTerminateProcess (code, -1);
}
