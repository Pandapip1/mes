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

/* Calling ntdll from code a real C compiler compiled.
 *
 * ntdll is stdcall: the routine pops its own arguments before it returns.
 * The two compilers that build this library before TinyCC exists have to be
 * talked around that -- MesCC adds back what it pushed, and M2-Planet needs
 * its arguments written backwards -- and lib/windows/x86-mes-mescc/ntcall.c
 * and lib/windows/x86-mes-m2/ntcall.c are a page of assembly each for it.
 *
 * TinyCC simply knows the convention.  __attribute__ ((stdcall)) on the
 * pointer type is the whole of it: the arguments go out in reverse, so the
 * first one lands nearest the stack pointer where a stdcall callee looks for
 * it, and nothing is added back afterwards because the frame pointer is what
 * restores the stack.  So these are ordinary C, and stay here rather than
 * moving up into lib/windows/ only because the two compilers below cannot
 * say the same thing.
 *
 * Each __ntcallN takes the routine and N arguments and calls it with them in
 * the order they are written.
 *
 * GENERATED-LOOKING BUT HAND-MAINTAINED: the arities differ only in how many
 * arguments they pass.  11 is the largest ntdll routine this C ever calls
 * (NtCreateFile).
 */

#include <windows/ntcall.h>

typedef int (__attribute__ ((stdcall)) *__ntfn0) (void);

int
__ntcall0 (int fn)
{
  __ntfn0 f;

  f = (__ntfn0) fn;
  return f ();
}

typedef int (__attribute__ ((stdcall)) *__ntfn1) (int);

int
__ntcall1 (int fn, int a1)
{
  __ntfn1 f;

  f = (__ntfn1) fn;
  return f (a1);
}

typedef int (__attribute__ ((stdcall)) *__ntfn2) (int, int);

int
__ntcall2 (int fn, int a1, int a2)
{
  __ntfn2 f;

  f = (__ntfn2) fn;
  return f (a1, a2);
}

typedef int (__attribute__ ((stdcall)) *__ntfn3) (int, int, int);

int
__ntcall3 (int fn, int a1, int a2, int a3)
{
  __ntfn3 f;

  f = (__ntfn3) fn;
  return f (a1, a2, a3);
}

typedef int (__attribute__ ((stdcall)) *__ntfn4) (int, int, int, int);

int
__ntcall4 (int fn, int a1, int a2, int a3, int a4)
{
  __ntfn4 f;

  f = (__ntfn4) fn;
  return f (a1, a2, a3, a4);
}

typedef int (__attribute__ ((stdcall)) *__ntfn5) (int, int, int, int, int);

int
__ntcall5 (int fn, int a1, int a2, int a3, int a4, int a5)
{
  __ntfn5 f;

  f = (__ntfn5) fn;
  return f (a1, a2, a3, a4, a5);
}

typedef int (__attribute__ ((stdcall)) *__ntfn6) (int, int, int, int, int,
                                                  int);

int
__ntcall6 (int fn, int a1, int a2, int a3, int a4, int a5, int a6)
{
  __ntfn6 f;

  f = (__ntfn6) fn;
  return f (a1, a2, a3, a4, a5, a6);
}

typedef int (__attribute__ ((stdcall)) *__ntfn7) (int, int, int, int, int,
                                                  int, int);

int
__ntcall7 (int fn, int a1, int a2, int a3, int a4, int a5, int a6, int a7)
{
  __ntfn7 f;

  f = (__ntfn7) fn;
  return f (a1, a2, a3, a4, a5, a6, a7);
}

typedef int (__attribute__ ((stdcall)) *__ntfn8) (int, int, int, int, int,
                                                  int, int, int);

int
__ntcall8 (int fn, int a1, int a2, int a3, int a4, int a5, int a6, int a7,
           int a8)
{
  __ntfn8 f;

  f = (__ntfn8) fn;
  return f (a1, a2, a3, a4, a5, a6, a7, a8);
}

typedef int (__attribute__ ((stdcall)) *__ntfn9) (int, int, int, int, int,
                                                  int, int, int, int);

int
__ntcall9 (int fn, int a1, int a2, int a3, int a4, int a5, int a6, int a7,
           int a8, int a9)
{
  __ntfn9 f;

  f = (__ntfn9) fn;
  return f (a1, a2, a3, a4, a5, a6, a7, a8, a9);
}

typedef int (__attribute__ ((stdcall)) *__ntfn10) (int, int, int, int, int,
                                                   int, int, int, int, int);

int
__ntcall10 (int fn, int a1, int a2, int a3, int a4, int a5, int a6, int a7,
            int a8, int a9, int a10)
{
  __ntfn10 f;

  f = (__ntfn10) fn;
  return f (a1, a2, a3, a4, a5, a6, a7, a8, a9, a10);
}

typedef int (__attribute__ ((stdcall)) *__ntfn11) (int, int, int, int, int,
                                                   int, int, int, int, int,
                                                   int);

int
__ntcall11 (int fn, int a1, int a2, int a3, int a4, int a5, int a6, int a7,
            int a8, int a9, int a10, int a11)
{
  __ntfn11 f;

  f = (__ntfn11) fn;
  return f (a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11);
}
