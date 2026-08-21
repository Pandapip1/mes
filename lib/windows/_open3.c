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
#include <fcntl.h>
#include <errno.h>

/* Stands in for lib/linux/_open3.c.
 *
 * The POSIX flags are turned into the DesiredAccess and CreateDisposition
 * NtCreateFile wants.  Windows has no permission bits of the shape mask
 * carries, so mask is accepted and ignored -- see lib/windows/chmod.c, which
 * is the same answer at more length.
 *
 * The bookkeeping after the open is Mes's own and is the same as the Linux
 * one: a fresh descriptor has no pushed-back character and nothing buffered.
 */

int
_open3 (char const *file_name, int flags, int mask)
{
  int NtCreateFile;
  int *oa;
  int *iosb;
  int *handle;
  int access;
  int disposition;
  int rc;
  int r;

  oa = __ntobject (file_name);
  if (oa == 0)
    return -1;

  iosb = malloc (8);
  iosb[0] = 0;
  iosb[1] = 0;
  handle = malloc (4);
  handle[0] = 0;

  if ((flags & 3) == O_RDONLY)
    {
      access = 0x80100000;      /* GENERIC_READ | SYNCHRONIZE */
      disposition = 1;          /* FILE_OPEN: it has to be there already */
    }
  else
    {
      access = 0xc0100000;      /* GENERIC_READ | GENERIC_WRITE | SYNCHRONIZE */
      if ((flags & O_TRUNC) != 0 || (flags & O_CREAT) != 0)
        disposition = 5;        /* FILE_OVERWRITE_IF: made, or emptied */
      else
        disposition = 1;        /* FILE_OPEN */
    }

  NtCreateFile = __ntdll_resolve ("NtCreateFile");
  rc = __ntcall11 (NtCreateFile, handle, access, oa, iosb, 0, 0x80, 3,
                   disposition, 0x20, 0, 0);
  if (rc != 0)
    return -1;

  r = handle[0];
  __ungetc_init ();
  if (r > 2)
    {
      if (r >= __FILEDES_MAX)
        {
          errno = EMFILE;
          return -1;
        }
      __ungetc_clear (r);
      __buffered_read_clear (r);
    }
  return r;
}
