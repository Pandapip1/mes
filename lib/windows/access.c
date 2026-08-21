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

/* Stands in for lib/linux/access.c.
 *
 * F_OK is answered by whether the file is there, and W_OK by the read-only
 * attribute.  R_OK and X_OK have no Windows counterpart worth consulting: a
 * file that exists can be read, and one runs because of what is inside it
 * rather than because of a bit beside it.
 */

/* include/unistd.h is not part of this build; access is the only caller. */
#define F_OK 0
#define X_OK 1
#define W_OK 2
#define R_OK 4

int
access (char const *file_name, int how)
{
  int NtQueryAttributesFile;
  int *oa;
  int *basic;
  int i;

  oa = __ntobject (file_name);
  if (oa == 0)
    return -1;

  /* FILE_BASIC_INFORMATION: four timestamps and then FileAttributes */
  basic = malloc (40);
  i = 0;
  while (i < 10)
    {
      basic[i] = 0;
      i = i + 1;
    }

  NtQueryAttributesFile = __ntdll_resolve ("NtQueryAttributesFile");
  if (__ntcall2 (NtQueryAttributesFile, oa, basic) != 0)
    return -1;

  i = basic[8];                 /* FileAttributes */
  if ((how & W_OK) != 0 && (i & 1) != 0)      /* FILE_ATTRIBUTE_READONLY */
    return -1;
  return 0;
}
