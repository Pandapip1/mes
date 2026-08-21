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
#include <sys/types.h>
#include <sys/wait.h>

/* Stands in for lib/linux/waitpid.c.
 *
 * A pid here is the process handle __spawn handed back, the way a file
 * descriptor is a file handle.  options is accepted and ignored: WNOHANG
 * would be a zero timeout on the wait below, and nothing here asks for it.
 *
 * The status goes back in the shape POSIX put it in, with the exit code in
 * the second byte up, so a caller's WEXITSTATUS shifts it down and gets what
 * the child returned.  Nothing here can be killed by a signal, so the low
 * byte, where that would be reported, is always zero.
 */

pid_t
waitpid (pid_t pid, int *status_ptr, int options)
{
  int NtWaitForSingleObject;
  int NtQueryInformationProcess;
  int *basic;
  int i;
  int rc;

  if (pid <= 0)
    return -1;

  NtWaitForSingleObject = __ntdll_resolve ("NtWaitForSingleObject");
  rc = __ntcall3 (NtWaitForSingleObject, pid, 0, 0);
  if (rc < 0)
    return -1;

  /* PROCESS_BASIC_INFORMATION, class 0, with ExitStatus first */
  basic = malloc (32);
  i = 0;
  while (i < 8)
    {
      basic[i] = 0;
      i = i + 1;
    }

  NtQueryInformationProcess = __ntdll_resolve ("NtQueryInformationProcess");
  rc = __ntcall5 (NtQueryInformationProcess, pid, 0, basic, 24, 0);
  if (rc != 0)
    return -1;

  if (status_ptr != 0)
    {
      i = basic[0];
      status_ptr[0] = 256 * i;
    }
  return pid;
}
