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

/* <string.h> has declared this for as long as it has declared strcmp, with
 * nothing behind it.  TinyCC's PE backend is the first thing in the
 * bootstrap to call it: tccpe.c reads a .def file's LIBRARY and EXPORTS
 * keywords case-insensitively, through its own stricmp/strnicmp macros. */

int
strcasecmp (char const *a, char const *b)
{
  while (a[0] != 0 && b[0] != 0 && tolower (a[0]) == tolower (b[0]))
    {
      a = a + 1;
      b = b + 1;
    }

  return tolower (a[0]) - tolower (b[0]);
}
