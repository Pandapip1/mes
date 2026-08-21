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
#include <time.h>
#include <sys/time.h>
#include <stdlib.h>

/* Stands in for lib/m2/time.c, which reaches past the libc to int $0x80 and
 * so cannot be shared.  This is the same function over gettimeofday. */

char *__tv;

long
time (long *result)
{
  struct timeval *tv;

  if (__tv == 0)
    __tv = malloc (8);
  tv = __tv;

  if (gettimeofday (tv, 0) != 0)
    return -1;
  if (result != 0)
    result[0] = tv->tv_sec;
  return tv->tv_sec;
}
