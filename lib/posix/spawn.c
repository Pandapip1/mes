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
#include <mes/lib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

/* Start a program, wait for it, and answer with what it exited with.
 *
 * On a system with fork this is the three calls a shell has always made, in
 * the order it has always made them, and there is nothing here a caller could
 * not have written itself.  It exists because on Windows there is no fork to
 * write it with -- see lib/windows/spawn.c, which reaches the same result by
 * a different route -- and system* would rather ask for the operation than
 * for the primitives.
 *
 * The status is the whole POSIX word, exit code in the second byte up, as
 * waitpid reports it: mes/module/mes/posix.mes hands it straight back to a
 * caller that will use status:exit-val on it.
 */

int
spawn (char const *file_name, char *const argv[])
{
  int pid;
  int status;

  pid = fork ();
  if (pid < 0)
    return -1;
  if (pid == 0)
    {
      execve (file_name, argv, environ);
      _exit (127);
    }

  status = 0;
  if (waitpid (pid, &status, 0) < 0)
    return -1;
  return status;
}
