/* -*-comment-start: "//";comment-end:""-*-
 * GNU Mes --- Maxwell Equations of Software
 * Copyright © 2018,2019 Jan (janneke) Nieuwenhuizen <janneke@gnu.org>
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

#include <signal.h>
#include <stdlib.h>

void
abort (void)
{
  if (raise (SIGABRT) < 0) /* could not raise SIGABRT */
    /* Nothing on this system delivers the signal (Windows has no kill; see
       lib/stub/kill.c), so there is nothing to fail into by leaving SIGABRT
       undelivered -- exit the way a real SIGABRT death would look to
       whatever is watching the exit code, rather than the wild pointer
       write this used to be, which only ever produced an unreadable
       platform-specific crash instead of a diagnosable one. 128 + SIGABRT
       is the shell convention for "killed by this signal". */
    _exit (128 + SIGABRT);
}
