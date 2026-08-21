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

/* Stands in for lib/linux/close.c.  NtClose takes the handle and nothing
 * else; every kind of handle goes back the same way.
 *
 * The three standard streams are 0, 1 and 2 here as C says, and __handle
 * turns those into the handles the process was started with.  Closing one
 * of those is a program's own business -- a shell does it on purpose --
 * so nothing here refuses to. */

#include <windows/ntcall.h>
#include <windows/ntdll.h>
#include <mes/lib.h>

int
close (int filedes)
{
  int NtClose;
  int handle;

  handle = __handle (filedes);
  NtClose = __ntdll_resolve ("NtClose");
  if (__ntcall1 (NtClose, handle) < 0)
    return -1;
  return 0;
}
