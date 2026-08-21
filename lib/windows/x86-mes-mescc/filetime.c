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

/* The 64-bit arithmetic lib/windows/gettimeofday.c cannot write in C.
 *
 * The x86 div instruction divides the 64 bits in edx:eax by a 32-bit divisor,
 * and doing it twice -- high half first, remainder carried into the low half
 * in edx -- divides a 64-bit number exactly.
 *
 * 116444736000000000 is the count of 100-nanosecond units between the two
 * epochs, and 0x019DB1DE 0xD53E8000 is that number as two halves.  After
 * subtracting it the value is about 1.8e16, so the high half of the quotient
 * is zero and only the low half is kept: seconds fit in 32 bits until 2038,
 * which is a problem this shares with every other 32-bit system and not one
 * to solve here.
 *
 * The same routine as lib/windows/x86-mes-m2/filetime.c, in MesCC's assembly
 * dialect and the other way up: MesCC is cdecl, so the first argument is at
 * ebp+8 and the second at ebp+12 rather than below the frame pointer.  esi is
 * saved and restored because MesCC did not offer it; the mnemonics are
 * DEFINEd in lib/x86-mes/x86.M1, which is the only macro file MesCC gives M1.
 */

void
__filetime_to_timeval (int *ft, int *tv)
{
  asm ("push___%esi");

  asm ("mov____0x8(%ebp),%esi !8");      /* ft */
  asm ("mov____(%esi),%eax");            /* low half */
  asm ("mov____0x8(%esi),%edx !4");      /* high half */
  asm ("sub____$i32,%eax %0xD53E8000");  /* 1601 to 1970, low */
  asm ("sbb____$i32,%edx %0x019DB1DE");  /* and high, with the borrow */

  asm ("push___%eax");                   /* the low half, for the second divide */
  asm ("mov____%edx,%eax");
  asm ("mov____$i32,%edx %0");
  asm ("mov____$i32,%ecx %10000000");    /* 100ns units in a second */
  asm ("div___%ecx");                    /* high half: quotient discarded, */
  asm ("pop____%eax");                   /* remainder stays in edx */
  asm ("div___%ecx");                    /* eax = seconds, edx = the rest */

  asm ("mov____0x8(%ebp),%esi !12");     /* tv */
  asm ("mov____%eax,(%esi)");            /* tv_sec */
  asm ("mov____%edx,%eax");
  asm ("mov____$i32,%edx %0");
  asm ("mov____$i32,%ecx %10");          /* 100ns units in a microsecond */
  asm ("div___%ecx");
  asm ("mov____%eax,0x8(%esi) !4");      /* tv_usec */

  asm ("pop____%esi");
}
