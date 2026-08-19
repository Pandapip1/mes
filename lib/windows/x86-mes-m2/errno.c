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

/* lib/linux/x86-mes-m2/syscall.c defines errno alongside the _sys_call
 * family, because on Linux errno is what a negative syscall return becomes.
 * There is no syscall here and no _sys_call for lib/windows/ to wrap -- each
 * of those files calls ntdll directly -- but errno still has to live
 * somewhere, so it lives here.
 *
 * Nothing in lib/windows/ sets it yet.  An NTSTATUS is not an errno and the
 * translation between them is a table of several hundred entries
 * (RtlNtStatusToDosError, and then another table after that); a call that
 * fails here returns -1 and says nothing more.  That is worse than Linux and
 * it is worth knowing about, rather than papering over with a number that
 * would be wrong.
 */

#include <errno.h>

int errno;
