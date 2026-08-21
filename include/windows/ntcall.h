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

#ifndef __MES_WINDOWS_NTCALL_H
#define __MES_WINDOWS_NTCALL_H

/* One way to call an ntdll routine, whichever compiler is building this.
 *
 * ntdll is stdcall, and the two compilers that build lib/windows/ disagree
 * about what that costs.  M2-Planet can call it directly, provided the
 * arguments are written in reverse, nothing computed appears among them, and
 * the caller knows the routine pops them.  MesCC cannot call it directly at
 * all: it adds back what it pushed, which against a callee that already
 * popped walks the stack pointer up by 4n on every call.
 *
 * So neither calls it directly.  A caller says
 *
 *     rc = __ntcall9 (NtWriteFile, handle, 0, 0, 0, iosb, buffer, size, 0, 0);
 *
 * with the arguments in the order the routine documents, and the per-compiler
 * implementation -- lib/windows/x86-mes-mescc/ntcall.c and
 * lib/windows/x86-mes-m2/ntcall.c -- puts them where that compiler must and
 * leaves the stack where it found it.  That is what lets the files in
 * lib/windows/ above this one be ordinary C that either compiler can build,
 * rather than one dialect's.
 *
 * The arity is in the name because neither compiler has varargs here.
 */

int __ntcall0 (int fn);
int __ntcall1 (int fn, int a1);
int __ntcall2 (int fn, int a1, int a2);
int __ntcall3 (int fn, int a1, int a2, int a3);
int __ntcall4 (int fn, int a1, int a2, int a3, int a4);
int __ntcall5 (int fn, int a1, int a2, int a3, int a4, int a5);
int __ntcall6 (int fn, int a1, int a2, int a3, int a4, int a5, int a6);
int __ntcall7 (int fn, int a1, int a2, int a3, int a4, int a5, int a6, int a7);
int __ntcall8 (int fn, int a1, int a2, int a3, int a4, int a5, int a6, int a7,
               int a8);
int __ntcall9 (int fn, int a1, int a2, int a3, int a4, int a5, int a6, int a7,
               int a8, int a9);
int __ntcall10 (int fn, int a1, int a2, int a3, int a4, int a5, int a6,
                int a7, int a8, int a9, int a10);
int __ntcall11 (int fn, int a1, int a2, int a3, int a4, int a5, int a6,
                int a7, int a8, int a9, int a10, int a11);

#endif /* __MES_WINDOWS_NTCALL_H */
