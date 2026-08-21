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
#include <windows/ntdll.h>
#include <mes/lib.h>
#include <sys/time.h>

/* Stands in for lib/linux/gettimeofday.c.
 *
 * Windows counts time in units of 100 nanoseconds since the start of 1601,
 * and POSIX in seconds and microseconds since the start of 1970.  Getting
 * from one to the other is a 64-bit subtraction and a 64-bit division on a
 * machine with 32-bit registers, and neither compiler here has a 64-bit type
 * to write that in -- so __filetime_to_timeval is assembly, and lives in the
 * directory of whichever compiler is building this.  Everything else about
 * the call is ordinary C and stays here.
 */

void __filetime_to_timeval (int *ft, int *tv);

int
gettimeofday (struct timeval *tv, struct timezone *tz)
{
  int (*NtQuerySystemTime) (int);
  int *ft;

  ft = malloc (8);
  ft[0] = 0;
  ft[1] = 0;

  NtQuerySystemTime = __ntdll_resolve ("NtQuerySystemTime");
  if (NtQuerySystemTime (ft) != 0)
    return -1;

  __filetime_to_timeval (ft, tv);
  return 0;
}
