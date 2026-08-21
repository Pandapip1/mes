/* -*-comment-start: "//";comment-end:""-*-
 * GNU Mes --- Maxwell Equations of Software
 * Copyright © 2019 Jan (janneke) Nieuwenhuizen <janneke@gnu.org>
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

/* There is one user here, and it is 0.

   A stub in the sense lib/stub/ means it: it is here so that a program
   naming it links, and it says so under __mes_debug rather than pretending
   to have done something. */

#include <mes/lib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>

uid_t
geteuid ()
{
  static int stub = 0;
  if (__mes_debug () && !stub)
    eputs ("geteuid stub\n");
  stub = 1;
  errno = 0;
  return 0;
}
