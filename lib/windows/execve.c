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

/* Stands in for lib/m2/execve.c, which reaches past the libc to int $0x80.
 *
 * Nothing on Windows replaces a running image, so __spawn starts the program
 * as a child and execve waits for it and exits as it did.  To anything
 * watching from outside that is what execve does: the call does not return,
 * and the process ends with the child's status.  What is different is that
 * this process stays alive, doing nothing but waiting, while the child runs.
 *
 * RtlCreateProcessParameters builds the block the child's PEB will point at,
 * with this process's three standard handles copied into it so the child
 * reads and writes wherever this one does.  RtlCreateUserProcess makes the
 * process with its first thread suspended, which is why NtResumeThread has to
 * be called before anything happens.
 *
 * Windows hands a child one string rather than a vector, so argv is joined
 * into one here and crt1.M1 splits it again at the other end.  Both halves
 * follow the rule CommandLineToArgvW defines, which is the one every Windows
 * program is parsed by, so an argument survives the round trip whatever is in
 * it.  See next_token in lib/m2/x86/ntdll-i386.hex2 for the other side.
 */

/* The closing quote of a quoted argument.  A function of its own because
 * __quote_arg reaches the end from two places. */
int
__quote_close (char *dst, int at)
{
  dst[at] = '"';
  return at + 1;
}

/* One argument, written into dst at `at` the way CommandLineToArgvW will read
 * it back, and where the next one would go.
 *
 * An argument with nothing awkward in it is written as it stands.  One with a
 * space, a tab or a quote is wrapped in quotes, and two things then have to be
 * escaped inside: a quote becomes \", and any run of backslashes about to be
 * followed by a quote -- the escaped one, or the one that closes the argument
 * -- is doubled, so the reader can tell a backslash that is text from one
 * protecting the quote after it.  A run anywhere else is left alone.
 *
 * C:\dir\ is the case that makes this necessary rather than pedantic: written
 * naively it would end ...dir\", and the reader would take that backslash to
 * be protecting the closing quote and swallow the rest of the command line. */
int
__quote_arg (char *dst, int at, char *arg)
{
  int i;
  int n;
  int plain;

  plain = 1;
  if (arg[0] == 0)
    plain = 0;
  i = 0;
  while (arg[i] != 0)
    {
      if (arg[i] == ' ' || arg[i] == '\t' || arg[i] == '"')
        plain = 0;
      i = i + 1;
    }

  if (plain != 0)
    {
      i = 0;
      while (arg[i] != 0)
        {
          dst[at] = arg[i];
          at = at + 1;
          i = i + 1;
        }
      return at;
    }

  dst[at] = '"';
  at = at + 1;
  i = 0;
  while (arg[i] != 0)
    {
      n = 0;
      while (arg[i] == '\\')
        {
          n = n + 1;
          i = i + 1;
        }

      if (arg[i] == 0)
        n = 2 * n;              /* the run runs into the closing quote */
      else if (arg[i] == '"')
        n = 2 * n + 1;          /* and the odd one protects the quote */

      while (n > 0)
        {
          dst[at] = '\\';
          at = at + 1;
          n = n - 1;
        }

      if (arg[i] == 0)
        return __quote_close (dst, at);

      dst[at] = arg[i];
      at = at + 1;
      i = i + 1;
    }
  return __quote_close (dst, at);
}

/* argv as the single command line Windows gives a child.  Room for twice each
 * argument plus its quotes, which is the worst an argument of nothing but
 * backslashes and quotes could come to. */
char *
__cmdline (char **argv)
{
  int n;
  int i;
  int j;
  char *out;
  int at;
  char *a;

  n = 0;
  i = 0;
  while (argv[i] != 0)
    {
      a = argv[i];
      j = 0;
      while (a[j] != 0)
        j = j + 1;
      n = n + 2 * j + 3;
      i = i + 1;
    }

  out = malloc (n + 1);
  at = 0;
  i = 0;
  while (argv[i] != 0)
    {
      if (at != 0)
        {
          out[at] = ' ';
          at = at + 1;
        }
      at = __quote_arg (out, at, argv[i]);
      i = i + 1;
    }
  out[at] = 0;
  return out;
}

/* env as the block Windows wants: the entries one after another in UTF-16,
 * each ended by a zero, and a second zero after the last.  0 in means 0 out,
 * which tells RtlCreateProcessParameters to give the child this process's
 * environment rather than a new one. */
char *
__envblock (char **env)
{
  int n;
  int i;
  int j;
  char *w;
  int at;
  char *e;

  if (env == 0)
    return 0;

  n = 0;
  i = 0;
  while (env[i] != 0)
    {
      e = env[i];
      j = 0;
      while (e[j] != 0)
        j = j + 1;
      n = n + j + 1;
      i = i + 1;
    }

  w = malloc (2 * n + 4);
  i = 0;
  while (i < 2 * n + 4)
    {
      w[i] = 0;
      i = i + 1;
    }

  at = 0;
  i = 0;
  while (env[i] != 0)
    {
      e = env[i];
      j = 0;
      while (e[j] != 0)
        {
          w[2 * at] = e[j];
          at = at + 1;
          j = j + 1;
        }
      at = at + 1;
      i = i + 1;
    }
  return w;
}

/* Start a program.  What comes back is a handle to it, which waitpid takes,
 * or -1. */
int
__spawn (char const *file_name, char **argv, char **env)
{
  int (*RtlCreateProcessParameters) (int, int, int, int, int, int, int, int,
                                     int, int);
  int (*RtlCreateUserProcess) (int, int, int, int, int, int, int, int, int,
                               int);
  int (*NtResumeThread) (int, int);
  int *oa;
  int *ntpath;
  int *image;
  int *cmd;
  char *line;
  char *block;
  int *out;
  int *params;
  int *info;
  int *slot;
  int thread;
  int i;
  int rc;

  oa = __ntobject (file_name);
  if (oa == 0)
    return -1;
  ntpath = oa[2];               /* the UNICODE_STRING __ntobject made */

  line = __cmdline (argv);
  image = __dosustring (file_name);
  cmd = __dosustring (line);
  block = __envblock (env);
  out = malloc (4);
  out[0] = 0;

  RtlCreateProcessParameters = __ntdll (NT_MAKEPARAMS);
  /* forwards: RtlCreateProcessParameters (out, image, 0, 0, cmd, block,
   *                                       0, 0, 0, 0) */
  rc = RtlCreateProcessParameters (0, 0, 0, 0, block, cmd, 0, 0, image, out);
  if (rc != 0)
    return -1;
  params = out[0];

  /* hStdInput, hStdOutput and hStdError are at 0x18, 0x1c and 0x20 into the
   * block: the same three words __stdslot points into here.
   *
   * This works under wine and does not work on Windows, and the difference is
   * not understood -- see the head of stage0-pe32's x86/M2libc-windows/
   * process.c, where the same code is and where what was measured is written
   * down.  A child still runs, still reads and writes files it opens itself,
   * and still reports its exit status; only handles it was given rather than
   * opened are affected.  Nothing in Mes spawns anything, so nothing here
   * depends on it yet. */
  slot = __stdslot (0);
  params[6] = slot[0];
  slot = __stdslot (1);
  params[7] = slot[0];
  slot = __stdslot (2);
  params[8] = slot[0];

  /* RTL_USER_PROCESS_INFORMATION: Length, Process, Thread, a CLIENT_ID and a
   * SECTION_IMAGE_INFORMATION, 68 bytes altogether.  Only the first three
   * words are read here; the room is larger because being generous costs
   * nothing. */
  info = malloc (128);
  i = 0;
  while (i < 32)
    {
      info[i] = 0;
      i = i + 1;
    }
  info[0] = 68;

  RtlCreateUserProcess = __ntdll (NT_CREATEPROC);
  /* forwards: RtlCreateUserProcess (ntpath, OBJ_CASE_INSENSITIVE, params,
   *                                 0, 0, 0, TRUE, 0, 0, info) */
  rc = RtlCreateUserProcess (info, 0, 0, 1, 0, 0, 0, params, 0x40, ntpath);
  if (rc != 0)
    return -1;

  NtResumeThread = __ntdll (NT_RESUME);
  thread = info[2];
  /* forwards: NtResumeThread (thread, 0) */
  NtResumeThread (0, thread);

  return info[1];
}

int
execve (char const *file_name, char **argv, char **env)
{
  int *status;
  int pid;

  pid = __spawn (file_name, argv, env);
  if (pid <= 0)
    return -1;

  status = malloc (4);
  status[0] = 0;
  if (waitpid (pid, status, 0) < 0)
    return -1;

  _exit (status[0] / 256);
  return -1;
}
