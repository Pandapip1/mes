/* -*-comment-start: "//";comment-end:""-*-
 * GNU Mes --- Maxwell Equations of Software
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

#include <ctype.h>
#include <string.h>

/* strcasecmp's counterpart, and strncmp's: see lib/string/strcasecmp.c for
 * what in the bootstrap first asks for either of them. */

int
strncasecmp (char const *a, char const *b, size_t size)
{
  if (size == 0)
    return 0;

  while (size > 1 && a[0] != 0 && b[0] != 0
         && tolower (a[0]) == tolower (b[0]))
    {
      a = a + 1;
      b = b + 1;
      size = size - 1;
    }

  return tolower (a[0]) - tolower (b[0]);
}
