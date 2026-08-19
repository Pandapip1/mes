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

/* Stands in for lib/linux/_getcwd.c.  RtlGetCurrentDirectory_U counts in
 * bytes both ways, and when the buffer is too small it returns the size it
 * wanted rather than filling it. */

char *
_getcwd (char *buffer, size_t size)
{
  int (*RtlGetCurrentDirectory_U) (int, int);
  char *w;
  int room;
  int bytes;
  int n;
  int i;

  w = malloc (2 * size + 2);
  room = 2 * size;              /* not in the argument list: see ntdll.c */

  RtlGetCurrentDirectory_U = __ntdll (NT_GETCWD);
  /* forwards: RtlGetCurrentDirectory_U (room, w) */
  bytes = RtlGetCurrentDirectory_U (w, room);
  if (bytes == 0)
    return 0;

  n = bytes / 2;
  if (n >= size)
    return 0;

  i = 0;
  while (i < n)
    {
      buffer[i] = w[2 * i];
      i = i + 1;
    }
  buffer[n] = 0;
  return buffer;
}
