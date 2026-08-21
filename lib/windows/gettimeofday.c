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
 * machine with 32-bit registers, which is why the conversion is assembly:
 * the x86 div instruction divides the 64 bits in edx:eax by a 32-bit divisor,
 * and doing it twice -- high half first, remainder carried into the low half
 * in edx -- divides a 64-bit number exactly.
 *
 * 116444736000000000 is the count between the two epochs, and 0x019DB1DE
 * 0xD53E8000 is that number as two halves.  After subtracting it the value is
 * about 1.8e16, so the high half of the quotient is zero and only the low
 * half is kept: seconds fit in 32 bits until 2038, which is a problem this
 * shares with every other 32-bit system and not one to solve here.
 */

void
__filetime_to_timeval (int *ft, int *tv)
{
  asm ("mov____0x8(%ebp),%esi !-4");     /* ft */
  asm ("mov_eax,[esi]");                 /* low half */
  asm ("mov_edx,[esi+BYTE] !4");         /* high half */
  asm ("sub_eax, %0xD53E8000");          /* 1601 to 1970, low */
  asm ("sbb_edx, %0x019DB1DE");          /* and high, with the borrow */

  asm ("push_eax");                      /* the low half, for the second divide */
  asm ("mov_eax,edx");
  asm ("mov_edx, %0");
  asm ("mov_ecx, %10000000");            /* 100ns units in a second */
  asm ("div_ecx");                       /* high half: quotient discarded, */
  asm ("pop_eax");                       /* remainder stays in edx */
  asm ("div_ecx");                       /* eax = seconds, edx = the rest */

  asm ("mov____0x8(%ebp),%esi !-8");     /* tv */
  asm ("mov_[esi],eax");                 /* tv_sec */
  asm ("mov_eax,edx");
  asm ("mov_edx, %0");
  asm ("mov_ecx, %10");                  /* 100ns units in a microsecond */
  asm ("div_ecx");
  asm ("mov_[esi+BYTE],eax !4");         /* tv_usec */
}

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
