/* -*-comment-start: "//";comment-end:""-*-
 * GNU Mes --- Maxwell Equations of Software
 * Copyright © 2018 Jan (janneke) Nieuwenhuizen <janneke@gnu.org>
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

#include <string.h>

/* Null when no character of string is in stopset -- C99 7.21.5.4p3 -- and
 * not, as this used to, a pointer to string's own terminator.  That pointer
 * is not null, so every caller that asks "is there one of these in there?"
 * was told yes about every string.  TinyCC's archiver asks exactly that,
 * of the mode letters it does not implement: `tcc -ar rcs' found an "a" in
 * "rcs" and refused to build any library at all. */
char *
strpbrk (char const *string, char const *stopset)
{
  char *p = (char *) string;
  while (*p)
    {
      if (strchr (stopset, *p))
        return p;
      p = p + 1;
    }
  return 0;
}
