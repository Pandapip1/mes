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

/* Stands in for lib/linux/_read.c.  On Linux _read is the bare SYS_read and
 * read is the same call; here read (lib/windows/read.c) is already the bare
 * NtReadFile with nothing between, so this is that. */

#include <mes/lib.h>
#include <unistd.h>

ssize_t
_read (int filedes, void *buffer, size_t size)
{
  return read (filedes, buffer, size);
}
