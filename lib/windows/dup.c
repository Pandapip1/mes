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

/* Stands in for lib/linux/dup.c.  NtDuplicateObject copies a handle within
 * this process; DUPLICATE_SAME_ACCESS is 2, and -1 is the pseudo-handle for
 * the current process, used for both source and target. */

int
dup (int old)
{
  int (*NtDuplicateObject) (int, int, int, int, int, int, int);
  int *out;
  int handle;
  int rc;

  out = malloc (4);
  out[0] = 0;
  handle = __handle (old);

  NtDuplicateObject = __ntdll_resolve ("NtDuplicateObject");
  /* forwards: NtDuplicateObject (-1, handle, -1, out, 0, 0,
   *                              DUPLICATE_SAME_ACCESS) */
  rc = NtDuplicateObject (2, 0, 0, out, -1, handle, -1);
  if (rc != 0)
    return -1;
  return out[0];
}
