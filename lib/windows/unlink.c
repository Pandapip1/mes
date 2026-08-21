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
#include <windows/ntcall.h>
#include <windows/ntdll.h>
#include <mes/lib.h>

/* Stands in for lib/linux/unlink.c.  NtDeleteFile opens the file for delete
 * and marks it gone in one step, so unlike the POSIX call this fails on a
 * file something else still has open: Windows will not unlink a file out from
 * under its readers the way Unix will. */

int
unlink (char const *file_name)
{
  int NtDeleteFile;
  int *oa;

  oa = __ntobject (file_name);
  if (oa == 0)
    return -1;

  NtDeleteFile = __ntdll_resolve ("NtDeleteFile");
  if (__ntcall1 (NtDeleteFile, oa) != 0)
    return -1;
  return 0;
}
