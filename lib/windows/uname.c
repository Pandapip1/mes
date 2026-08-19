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

/* Stands in for lib/linux/uname.c.
 *
 * RtlGetVersion is used rather than the documented GetVersionEx because it is
 * the one that does not lie: since Windows 8.1 the Win32 call reports what a
 * process's manifest asks for, and tells an unmanifested program it is on
 * 6.2, while this one reports what is actually running.
 *
 * nodename is left empty.  Windows keeps the computer name in the registry,
 * under \Registry\Machine\System\CurrentControlSet\Control\ComputerName,
 * which is two more ntdll routines and a key walk for a field nothing here
 * reads.
 */

int
uname (struct utsname *uts)
{
  int (*RtlGetVersion) (int);
  int *info;
  char *p;
  int i;
  int at;

  RtlGetVersion = __ntdll (NT_VERSION);

  /* RTL_OSVERSIONINFOW: five words and then 128 UTF-16 characters */
  info = malloc (276);
  i = 0;
  while (i < 69)
    {
      info[i] = 0;
      i = i + 1;
    }
  info[0] = 276;
  if (RtlGetVersion (info) != 0)
    return -1;

  p = uts;
  i = 0;
  while (i < 325)
    {
      p[i] = 0;
      i = i + 1;
    }

  __strput (uts->sysname, 0, "Windows_NT");
  __strput (uts->machine, 0, "i686");
  at = __strput (uts->release, 0, itoa (info[1]));
  at = __strput (uts->release, at, ".");
  __strput (uts->release, at, itoa (info[2]));
  __strput (uts->version, 0, itoa (info[3]));
  return 0;
}
