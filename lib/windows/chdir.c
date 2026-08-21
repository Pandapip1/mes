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

/* Stands in for lib/linux/chdir.c.  RtlSetCurrentDirectory_U takes the
 * directory as a UNICODE_STRING rather than an OBJECT_ATTRIBUTES, and it
 * accepts a DOS path, so __dosustring is the whole conversion -- there is
 * no \??\ prefix to put on and no object attributes to build. */

#include <windows/ntcall.h>
#include <windows/ntdll.h>
#include <mes/lib.h>

int
chdir (char const *file_name)
{
  int RtlSetCurrentDirectory_U;
  int *dir;

  dir = __dosustring (file_name);
  RtlSetCurrentDirectory_U = __ntdll_resolve ("RtlSetCurrentDirectory_U");
  if (__ntcall1 (RtlSetCurrentDirectory_U, dir) < 0)
    return -1;
  return 0;
}
