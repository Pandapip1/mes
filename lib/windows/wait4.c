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
#include <sys/types.h>
#include <sys/resource.h>

/* Stands in for lib/linux/wait4.c.  Upstream has waitpid call wait4 on
 * architectures with no waitpid syscall; here it is the other way round,
 * because there is no syscall either way and waitpid is the one that fits
 * what Windows offers.  rusage has no counterpart and is ignored.
 */

pid_t
wait4 (pid_t pid, int *status_ptr, int options, struct rusage *rusage)
{
  return waitpid (pid, status_ptr, options);
}
