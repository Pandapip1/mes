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
#include <stdio.h>
#include <sys/types.h>

/* Stands in for lib/linux/lseek.c.
 *
 * Windows keeps a file's position in the handle, the way Unix does, but there
 * is no one call that moves it by a relative amount: NtSetInformationFile
 * with FilePositionInformation takes an absolute offset, so SEEK_CUR and
 * SEEK_END have to read something first.  The current position comes from
 * NtQueryInformationFile with the same class; the length comes from
 * FileStandardInformation, whose EndOfFile is the second 64-bit field.
 *
 * Everything here is 32 bits wide.  A FILE_POSITION_INFORMATION is a 64-bit
 * offset and the high half is written as zero and read as though it were:
 * neither compiler in this bootstrap has a 64-bit type, and a file over 2GB
 * is not something the programs above this can produce.  A negative offset is
 * a real one -- SEEK_CUR backwards -- so the sign is carried into the high
 * half by hand.
 *
 * __buffered_read_clear is what makes lseek agree with getc: a read that was
 * buffered has already moved the handle further than the caller thinks, and
 * SEEK_CUR has to be measured from where the caller thinks it is.  The Linux
 * lseek does the same thing for the same reason.
 */

/* The class numbers NtQueryInformationFile and NtSetInformationFile take. */
#define FileStandardInformation 5
#define FilePositionInformation 14

off_t
_lseek (int filedes, off_t offset, int whence)
{
  int NtQueryInformationFile;
  int NtSetInformationFile;
  int *iosb;
  int *info;
  int handle;
  int rc;
  int at;

  handle = __handle (filedes);
  info = malloc (24);
  info[0] = 0;
  info[1] = 0;
  info[2] = 0;
  info[3] = 0;
  info[4] = 0;
  info[5] = 0;
  iosb = __iosb ();
  iosb[0] = 0;
  iosb[1] = 0;

  if (whence == SEEK_SET)
    at = offset;
  else
    {
      NtQueryInformationFile = __ntdll_resolve ("NtQueryInformationFile");
      if (whence == SEEK_CUR)
        {
          rc = __ntcall5 (NtQueryInformationFile, handle, iosb, info, 8,
                          FilePositionInformation);
          if (rc != 0)
            return -1;
          at = info[0] + offset;
        }
      else
        {
          /* FILE_STANDARD_INFORMATION: AllocationSize, then EndOfFile. */
          rc = __ntcall5 (NtQueryInformationFile, handle, iosb, info, 24,
                          FileStandardInformation);
          if (rc != 0)
            return -1;
          at = info[2] + offset;
        }
    }

  info[0] = at;
  if (at < 0)
    info[1] = -1;
  else
    info[1] = 0;

  NtSetInformationFile = __ntdll_resolve ("NtSetInformationFile");
  rc = __ntcall5 (NtSetInformationFile, handle, iosb, info, 8,
                  FilePositionInformation);
  if (rc != 0)
    return -1;
  return at;
}

off_t
lseek (int filedes, off_t offset, int whence)
{
  size_t skip = __buffered_read_clear (filedes);
  if (whence == SEEK_CUR)
    offset -= skip;
  return _lseek (filedes, offset, whence);
}
