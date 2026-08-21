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
#include <sys/stat.h>
#include <fcntl.h>

/* Stands in for lib/linux/chmod.c, and does nothing.
 *
 * On Unix the execute bit is what lets a file run at all.  On Windows nothing
 * consults it: an image runs because its PE header says it is an executable.
 * So reporting success is the honest answer to "is this file now executable",
 * not a stub standing in for a missing feature -- there is no feature.
 */

int
chmod (char const *file_name, mode_t mask)
{
  return 0;
}
