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
#include <mes/lib.h>

/* The 64-bit arithmetic lib/windows/gettimeofday.c cannot write in C -- for
 * the two compilers that have no 64-bit type.  This one does, so the file
 * that is a page of assembly for MesCC (lib/windows/x86-mes-mescc/
 * filetime.c) and for M2-Planet is four lines of C here, and the divides
 * that had to be written as two 32-bit `div' instructions in sequence are
 * calls to libtcc1's __udivdi3 and __umoddi3 instead.
 *
 * A FILETIME is the count of 100-nanosecond units since 1601, as two 32-bit
 * halves; 116444736000000000 of them separate that epoch from 1970's.  After
 * subtracting it the value is about 1.8e16, so the seconds fit in 32 bits
 * until 2038 -- a problem this shares with every other 32-bit system and not
 * one to solve here.
 */

void
__filetime_to_timeval (int *ft, int *tv)
{
  unsigned long long t;

  t = ((unsigned long long) (unsigned int) ft[1] << 32)
    | (unsigned long long) (unsigned int) ft[0];
  t = t - 116444736000000000ULL;
  tv[0] = (int) (t / 10000000ULL);       /* tv_sec */
  tv[1] = (int) (t % 10000000ULL / 10ULL); /* tv_usec */
}
