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

/* Stands in for lib/linux/x86-mes-gcc/_write.c.
 *
 * NtWriteFile takes nine arguments, of which this uses three: the handle, the
 * buffer and the length.  The Event, ApcRoutine and ApcContext are for
 * asynchronous writes, ByteOffset is NULL because every file here is opened
 * FILE_SYNCHRONOUS_IO_NONALERT and so keeps its own position, and Key is for
 * locked ranges.  What actually went out comes back in the IO_STATUS_BLOCK
 * rather than as the return value, which is an NTSTATUS.
 */

int
_write (int filedes, void *buffer, int size)
{
  int NtWriteFile;
  int *iosb;
  int handle;
  int rc;

  NtWriteFile = __ntdll_resolve ("NtWriteFile");
  iosb = __iosb ();
  iosb[0] = 0;
  iosb[1] = 0;
  handle = __handle (filedes);

  rc = __ntcall9 (NtWriteFile, handle, 0, 0, 0, iosb, buffer, size, 0, 0);
  if (rc < 0)
    return -1;
  return iosb[1];
}
