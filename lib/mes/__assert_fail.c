/* -*-comment-start: "//";comment-end:""-*-
 * GNU Mes --- Maxwell Equations of Software
 * Copyright © 2016,2017,2018,2019,2022 Jan (janneke) Nieuwenhuizen <janneke@gnu.org>
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

#include <mes/lib.h>
#include <assert.h>
#include <stdlib.h>
#include <signal.h>

/* Written as two nested ifs rather than `if (file && *file)`, because
 * M2-Planet does not short-circuit &&: it evaluates *file unconditionally
 * and ANDs the two results together, so the guard meant to skip a null
 * file dereferenced it instead -- every assertion failure that did not
 * pass a file name (assert_msg, the common case, always passes one as
 * null) crashed reporting itself. Measured: this is what was silently
 * eating the message and backtrace an unhandled Scheme exception had
 * already printed, on the one port where the second crash's own fallback
 * (see below) has no readable exit code and no debugger attached to see
 * it happen. */
void
__assert_fail (char const *msg, char const *file, unsigned line,
               char const *function)
{
  if (file)
    if (*file)
      {
        eputs (file);
        eputs (":");
      }
  if (line)
    {
      eputs (itoa (line));
      eputs (":");
    }
  if (function)
    if (*function)
      {
        eputs (function);
        eputs (":");
      }
  eputs ("assert fail: ");
  eputs (msg);
  eputs ("\n");
  /* Same fallback as abort() (lib/stdlib/abort.c) and the "abort" primitive
   * (src/posix.c's abort_) use, and for the same reason: forcing a crash
   * with an undefined write to address 0 loses whatever was just printed
   * above on any port where that write's failure mode is a debugger dump
   * rather than a core file next to a resumable shell. _exit with a real
   * status code preserves it. */
  _exit (134);
}
