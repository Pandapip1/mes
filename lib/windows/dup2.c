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

/* Stands in for lib/linux/dup2.c, and cannot mean quite what it does on Unix.
 *
 * There, a descriptor is a number the program chooses, so dup2 can put a file
 * at descriptor 1 and everything that writes to stdout goes there instead.
 * Here a descriptor is a handle, and the value is the kernel's to choose, so
 * there is no putting one at a particular number.
 *
 * What there is, and what redirection actually needs, is the three words in
 * the process parameters that say which handles are the standard ones.  So
 * dup2 works when the new descriptor is 0, 1 or 2 -- the case a shell wants --
 * and fails otherwise rather than pretending.
 */

int
dup2 (int old, int new)
{
  int *slot;
  int copy;

  if (new < 0 || new > 2)
    return -1;

  copy = dup (old);
  if (copy == -1)
    return -1;

  slot = __stdslot (new);
  slot[0] = copy;
  return new;
}
