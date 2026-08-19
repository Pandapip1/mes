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

/* What every other file in lib/windows/ stands on: how to reach an ntdll
 * routine, and how to hand it a filename.
 *
 * Windows has no syscall a program may make directly, so there is no
 * _sys_call here for lib/windows/ to be thin wrappers over the way
 * lib/linux/ is over int $0x80.  What there is instead:
 * lib/m2/x86/ntdll-i386.hex2 finds ntdll through the PEB before main runs and
 * resolves the routines this needs out of its export table by name, into a
 * table of addresses.  __ntdll hands one back by index, and the caller calls
 * it through a function pointer.
 *
 * Three things about those calls are not obvious and are not optional.
 *
 *   The arguments go in backwards.  M2-Planet pushes the first argument
 *   first, so it lands furthest from the stack pointer, and a stdcall callee
 *   wants the first argument nearest.  Every call below is written in reverse
 *   and carries a comment saying what it reads as forwards.
 *
 *   Nothing else has to be done about stdcall.  ntdll pops its own arguments,
 *   which would strand a caller that popped them again -- but M2-Planet saves
 *   the stack pointer before pushing and restores it from there afterwards,
 *   rather than adding back what it pushed.
 *
 *   No argument may touch EDX.  M2-Planet keeps the pointer it is about to
 *   call in EDX across the argument list and never puts it back, so an
 *   argument that writes EDX replaces the address about to be called and the
 *   program jumps to zero.  Three things write it: a function call; `*`, `/`
 *   or `%`, which compile to imul and idiv; and subscripting an array, which
 *   is a multiply by the element size even when the index is a constant.  So
 *   every argument below is a local or a constant, and anything computed is
 *   worked out into a local on the line before.
 *
 * The routine at each index is decided by lib/m2/x86/ntdll-i386.hex2, and
 * lib/windows/x86-mes-m2/ntdll.h names them.
 *
 * Filenames: ntdll takes UTF-16, inside an OBJECT_ATTRIBUTES around a
 * UNICODE_STRING, and for a file on disk it wants an NT path (\??\C:\...)
 * rather than the DOS path a program was given.
 * RtlDosPathNameToNtPathName_U does that conversion and __ntobject does the
 * rest.  Widening is a zero byte after each byte, which is right for ASCII
 * and wrong for everything else; a path with a character above 127 will not
 * survive, and this is not the layer that could say so.
 *
 * Nothing here frees anything.  Mes's malloc never gives memory back either.
 */

#include <windows/x86-mes-m2/ntdll.h>
#include <mes/lib.h>

/* An ntdll routine, by the index resolve_all put it at. */
void *
__ntdll (int slot)
{
  asm ("mov____0x8(%ebp),%eax !-4");
  asm ("sal_eax, !2");
  asm ("add_eax, &fn_table");
  asm ("mov_eax,[eax]");
}

/* Where one of the three standard handles is kept, so that dup2 can replace
 * it rather than only read it.  Redirection on Windows is exactly this: the
 * three words a child inherits are the three this points into. */
int *
__stdslot (int n)
{
  asm ("mov____0x8(%ebp),%eax !-4");
  asm ("sal_eax, !2");
  asm ("add_eax, %24");
  asm ("mov_ebx,eax");
  asm ("mov_eax,[fs:DWORD] %0x30");
  asm ("mov_eax,[eax+BYTE] !16");
  asm ("add_eax,ebx");
}

/* A file descriptor here is a Windows handle, with 0, 1 and 2 still meaning
 * the three standard streams as C says they do.  No real handle is that
 * small, so the two never collide and nothing above can tell. */
int
__handle (int filedes)
{
  int *slot;

  if (filedes > 2)
    return filedes;
  slot = __stdslot (filedes);
  return slot[0];
}

/* An ASCII string as the UTF-16 ntdll insists on. */
char *
__widen (char const *s)
{
  int n;
  int i;
  char *w;

  n = 0;
  while (s[n] != 0)
    n = n + 1;

  w = malloc (2 * n + 2);
  i = 0;
  while (i < 2 * n + 2)
    {
      w[i] = 0;
      i = i + 1;
    }
  i = 0;
  while (i < n)
    {
      w[2 * i] = s[i];
      i = i + 1;
    }
  return w;
}

/* A UNICODE_STRING over a DOS path: two words, the first holding Length and
 * MaximumLength as 16-bit halves, the second the widened text.  Both are byte
 * counts, and MaximumLength counts the terminator Length does not. */
int *
__dosustring (char const *path)
{
  int n;
  int *u;

  n = 0;
  while (path[n] != 0)
    n = n + 1;

  u = malloc (8);
  u[0] = 2 * n + 65536 * (2 * n + 2);
  u[1] = __widen (path);
  return u;
}

/* An OBJECT_ATTRIBUTES naming a file: six words, of which only the length,
 * the name and the attributes are ever anything but zero.  0 if the path
 * could not be made into an NT path at all. */
int *
__ntobject (char const *path)
{
  int (*RtlDosPathNameToNtPathName_U) (int, int, int, int);
  char *wide;
  int *name;
  int *oa;
  int i;

  RtlDosPathNameToNtPathName_U = __ntdll (NT_RTLPATH);

  name = malloc (8);
  name[0] = 0;
  name[1] = 0;
  wide = __widen (path);        /* not in the argument list: see above */

  /* forwards: RtlDosPathNameToNtPathName_U (wide, name, 0, 0) */
  if (RtlDosPathNameToNtPathName_U (0, 0, name, wide) == 0)
    return 0;

  oa = malloc (24);
  i = 0;
  while (i < 6)
    {
      oa[i] = 0;
      i = i + 1;
    }
  oa[0] = 24;                   /* Length: the size of this struct */
  oa[2] = name;                 /* ObjectName */
  oa[3] = 0x40;                 /* Attributes = OBJ_CASE_INSENSITIVE */
  return oa;
}

/* Copy a string into a fixed-size field of a struct, starting at `at`, and say
 * where the next piece would go.  Mes has strcpy, but not one that appends,
 * and lib/windows/uname.c has to build "10.0" out of two numbers. */
int
__strput (char *dst, int at, char const *src)
{
  int i;

  i = 0;
  while (src[i] != 0)
    {
      dst[at + i] = src[i];
      i = i + 1;
    }
  dst[at + i] = 0;
  return at + i;
}
