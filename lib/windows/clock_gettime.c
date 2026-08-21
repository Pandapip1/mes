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

/* Stands in for lib/linux/clock_gettime.c.
 *
 * clk_id is ignored.  NtQuerySystemTime is the wall clock, so this is
 * CLOCK_REALTIME whatever it is asked for; a monotonic clock would be
 * NtQueryPerformanceCounter and a different scale, and nothing here asks.
 * The difference from gettimeofday is nanoseconds rather than microseconds.
 */

int
clock_gettime (clockid_t clk_id, struct timespec *tp)
{
  struct timeval *tv;

  tv = malloc (8);
  if (gettimeofday (tv, 0) != 0)
    return -1;

  tp->tv_sec = tv->tv_sec;
  tp->tv_nsec = 1000 * tv->tv_usec;
  return 0;
}
