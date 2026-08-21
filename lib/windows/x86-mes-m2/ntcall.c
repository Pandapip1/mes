/* -*-comment-start: "//";comment-end:""-*-
 * GNU Mes --- Maxwell Equations of Software
 * Copyright © 2026 Gavin John <gavinnjohn@gmail.com>
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

/* Calling ntdll from code M2-Planet compiled.
 *
 * M2-Planet pushes the first argument first, so it lands furthest from the
 * stack pointer, and a stdcall callee wants the first argument nearest.
 * Every call here is therefore written in reverse -- which is what every
 * caller in lib/windows/ used to have to do for itself, with a comment
 * saying what it read as forwards.  Doing it once, here, is what lets those
 * files be ordinary C that MesCC can build too.
 *
 * Nothing else has to be done about stdcall for this compiler: ntdll pops
 * its own arguments, which would strand a caller that popped them again,
 * but M2-Planet saves the stack pointer before pushing and restores it from
 * there afterwards rather than adding back what it pushed.
 *
 * The third thing the callers used to have to watch -- that no argument may
 * touch EDX, because M2-Planet keeps the address it is about to call there
 * across the argument list -- is handled by construction now: every argument
 * below is a parameter of this function, so nothing is computed between
 * taking the address and making the call.
 */

#include <windows/ntcall.h>

int
__ntcall0 (int fn)
{
  int (*f) (void);

  f = fn;
  return f ();
}

int
__ntcall1 (int fn, int a1)
{
  int (*f) (int);

  f = fn;
  /* forwards: f (a1) */
  return f (a1);
}

int
__ntcall2 (int fn, int a1, int a2)
{
  int (*f) (int, int);

  f = fn;
  /* forwards: f (a1, a2) */
  return f (a2, a1);
}

int
__ntcall3 (int fn, int a1, int a2, int a3)
{
  int (*f) (int, int, int);

  f = fn;
  /* forwards: f (a1, a2, a3) */
  return f (a3, a2, a1);
}

int
__ntcall4 (int fn, int a1, int a2, int a3, int a4)
{
  int (*f) (int, int, int, int);

  f = fn;
  /* forwards: f (a1, a2, a3, a4) */
  return f (a4, a3, a2, a1);
}

int
__ntcall5 (int fn, int a1, int a2, int a3, int a4, int a5)
{
  int (*f) (int, int, int, int, int);

  f = fn;
  /* forwards: f (a1, a2, a3, a4, a5) */
  return f (a5, a4, a3, a2, a1);
}

int
__ntcall6 (int fn, int a1, int a2, int a3, int a4, int a5, int a6)
{
  int (*f) (int, int, int, int, int, int);

  f = fn;
  /* forwards: f (a1, a2, a3, a4, a5, a6) */
  return f (a6, a5, a4, a3, a2, a1);
}

int
__ntcall7 (int fn, int a1, int a2, int a3, int a4, int a5, int a6, int a7)
{
  int (*f) (int, int, int, int, int, int, int);

  f = fn;
  /* forwards: f (a1, a2, a3, a4, a5, a6, a7) */
  return f (a7, a6, a5, a4, a3, a2, a1);
}

int
__ntcall8 (int fn, int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8)
{
  int (*f) (int, int, int, int, int, int, int, int);

  f = fn;
  /* forwards: f (a1, a2, a3, a4, a5, a6, a7, a8) */
  return f (a8, a7, a6, a5, a4, a3, a2, a1);
}

int
__ntcall9 (int fn, int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8, int a9)
{
  int (*f) (int, int, int, int, int, int, int, int, int);

  f = fn;
  /* forwards: f (a1, a2, a3, a4, a5, a6, a7, a8, a9) */
  return f (a9, a8, a7, a6, a5, a4, a3, a2, a1);
}

int
__ntcall10 (int fn, int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8, int a9, int a10)
{
  int (*f) (int, int, int, int, int, int, int, int, int, int);

  f = fn;
  /* forwards: f (a1, a2, a3, a4, a5, a6, a7, a8, a9, a10) */
  return f (a10, a9, a8, a7, a6, a5, a4, a3, a2, a1);
}

int
__ntcall11 (int fn, int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8, int a9, int a10, int a11)
{
  int (*f) (int, int, int, int, int, int, int, int, int, int, int);

  f = fn;
  /* forwards: f (a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11) */
  return f (a11, a10, a9, a8, a7, a6, a5, a4, a3, a2, a1);
}
