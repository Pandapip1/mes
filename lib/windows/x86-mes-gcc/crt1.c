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

/* The Windows _start for a real, tcc-compiled program -- boot0 and every
 * round after it.
 *
 * lib/windows/x86-mes-mescc/crt1.M1 (with argv.M1 beside it) does this same
 * job for boot-mes.exe, but it is hand-assembled hex2-source: meant only to
 * be handed straight to hex2's own linker the way boot-mes.exe's own build
 * does it, not a real object file. A real linker -- tcc's own, here -- has
 * no way to read it, which is why boot0 needs this file at all: the same
 * startup, in C a real compiler can build into a real object file.
 *
 * Windows hands a program exactly one thing before main can run: one UTF-16
 * command line, reachable through the PEB (RtlUserProcessParameters at
 * PEB+0x10, ->CommandLine.Buffer at +0x44, ->Environment at +0x48 -- the
 * same struct lib/windows/ntdll.c's __stdslot already reads PEB+0x10 out
 * of). Splitting that into argc/argv and the environment array is what
 * argv.M1 did there; __next_arg and __init_env below do the identical
 * split, including Windows's own backslash-before-quote rule (an even run
 * of backslashes before a quote halves and the quote toggles quoting; an
 * odd run halves, rounding down, and the quote is literal instead), the one
 * part of this that is genuinely particular to Windows rather than to the
 * compiler.
 *
 * Unlike argv.M1, none of this needs a fixed offset measured from :PE_end.
 * That trick exists because hex2's own "linker" has no section or bss
 * concept at all -- everything past the file's real content is just
 * zero-filled address space a program divides up by hand. tcc is a real
 * compiler with a real .bss it sizes itself and the loader zero-fills the
 * ordinary way, so argv/envp storage below is nothing more than ordinary
 * static arrays.
 *
 * Only the low byte of each UTF-16 code unit is ever read or written --
 * right for ASCII, silently wrong above it, the same limit
 * lib/windows/ntdll.c's __widen already accepts for the same reason. */

#include <windows/ntcall.h>
#include <windows/ntdll.h>
#include <mes/lib.h>
#include <stdlib.h>

int __stdin;
int __stdout;
int __stderr;
char **environ;

int main (int argc, char **argv, char **envp);

/* 16384, not 256.  Windows caps a command line at 32767 characters, so no
 * argument can be shorter than one character and its separator and there
 * can never be many more than that many of them: this is a bound the system
 * already enforces, rather than a number chosen to fit the programs seen so
 * far.  256 was such a number and it was wrong -- ntlibc's archiver line
 * reached 272 arguments, argv stopped at 255, and `tcc -ar' archived the
 * first 251 objects, exited 0, and wrote a library missing sixteen members
 * with nothing said anywhere.  Pointers cost four bytes, so the array is
 * 64K of .bss and the question is closed rather than deferred.
 *
 * The byte buffers are sized past what a 32767-character command line and
 * a generous environment can hold, and -- unlike before -- are now checked
 * rather than trusted: see __next_arg and __init_env. */
#define __ARGV_MAX 16384
#define __ARG_BYTES_MAX 0x9000
#define __ENVP_MAX 4096
#define __ENV_BYTES_MAX 0x20000

char *__argv_buf[__ARGV_MAX];
char __arg_bytes[__ARG_BYTES_MAX];
char *__envp_buf[__ENVP_MAX];
char __env_bytes[__ENV_BYTES_MAX];

/* One past the end of __arg_bytes.  __next_arg refuses to write at or
 * beyond it, rather than running off the buffer into whatever .bss holds
 * next -- which is what it did before, unbounded. */
char *__arg_bytes_end;

/* One argument off the command line at *cursor, narrowed and unescaped into
 * *out_cursor, which this advances past the token and its NUL the way
 * *cursor is advanced past the token and its separator. Returns 0 (cursor
 * unmoved past trailing whitespace) when the command line has no more
 * tokens left, and -1 if the argument would not fit. */
int
__next_arg (char **cursor, char **out_cursor)
{
  char *p;
  char *o;
  int quoted;
  int backslashes;
  int i;

  p = *cursor;
  o = *out_cursor;

  while (*p == ' ' || *p == '\t')
    p += 2;

  if (*p == 0)
    {
      *cursor = p;
      return 0;
    }

  quoted = 0;
  while (*p)
    {
      if (!quoted && (*p == ' ' || *p == '\t'))
        break;

      if (*p == '\\')
        {
          backslashes = 0;
          while (*p == '\\')
            {
              backslashes = backslashes + 1;
              p += 2;
            }
          i = 0;
          if (*p == '"')
            {
              while (i < backslashes / 2)
                {
                  if (o >= __arg_bytes_end)
                    return -1;
                  *o++ = '\\';
                  i = i + 1;
                }
              if (backslashes % 2)
                {
                  if (o >= __arg_bytes_end)
                    return -1;
                  *o++ = '"';
                  p += 2;
                }
              else
                quoted = !quoted;
            }
          else
            {
              while (i < backslashes)
                {
                  if (o >= __arg_bytes_end)
                    return -1;
                  *o++ = '\\';
                  i = i + 1;
                }
            }
          continue;
        }

      if (*p == '"')
        {
          quoted = !quoted;
          p += 2;
          continue;
        }

      if (o >= __arg_bytes_end)
        return -1;
      *o++ = *p;
      p += 2;
    }

  if (o >= __arg_bytes_end)
    return -1;
  *o++ = 0;

  if (*p == ' ' || *p == '\t')
    p += 2;

  *cursor = p;
  *out_cursor = o;
  return 1;
}

/* The environment block: consecutive NUL-terminated entries, ended by an
 * entry that is itself empty. No backslash or quote handling here -- that
 * is a command-line-parsing rule, not a Windows-strings-in-general one. */
int
__init_env (char *env)
{
  int n;
  char *o;
  char *end;
  char c;

  n = 0;
  o = __env_bytes;
  end = __env_bytes + __ENV_BYTES_MAX;
  while (*env != 0 && n < __ENVP_MAX - 1)
    {
      __envp_buf[n] = o;
      for (;;)
        {
          if (o >= end)
            {
              __envp_buf[n] = 0;
              environ = __envp_buf;
              return -1;
            }
          c = *env;
          *o++ = c;
          env += 2;
          if (c == 0)
            break;
        }
      n = n + 1;
    }
  __envp_buf[n] = 0;
  environ = __envp_buf;

  /* Anything left means the block did not fit: too many entries, since the
   * byte case returned above. */
  if (*env != 0)
    return -1;
  return 0;
}

void
_start (void)
{
  int peb;
  int pp;
  char *cmdline;
  char *envblock;
  char *cursor;
  char *out;
  char *tok;
  int argc;
  int rc;
  int overflow;

  peb = __peb ();
  pp = *(int *) (peb + 16);            /* PEB->ProcessParameters */
  cmdline = *(char **) (pp + 68);      /* ->CommandLine.Buffer */
  envblock = *(char **) (pp + 72);     /* ->Environment */

  cursor = cmdline;
  out = __arg_bytes;
  __arg_bytes_end = __arg_bytes + __ARG_BYTES_MAX;
  argc = 0;
  overflow = 0;
  while (1)
    {
      if (argc >= __ARGV_MAX - 1)
        {
          overflow = 1;
          break;
        }
      tok = out;
      rc = __next_arg (&cursor, &out);
      if (rc == 0)
        break;
      if (rc < 0)
        {
          overflow = 1;
          break;
        }
      __argv_buf[argc] = tok;
      argc = argc + 1;
    }
  __argv_buf[argc] = 0;

  if (__init_env (envblock) < 0)
    overflow = 1;

  __init_io (argc, __argv_buf, __envp_buf);

  /* Said, not swallowed.  Running on with a truncated argv is how a `tcc
   * -ar' with 272 arguments came to archive 251 objects, exit 0, and write
   * a library missing sixteen members -- a failure that reaches its caller
   * as a link error in some other package, hours later.  Refusing here
   * costs a rebuild; continuing costs the afternoon. */
  if (overflow)
    {
      eputs ("crt1: command line or environment too large\n");
      _exit (1);
    }

  rc = main (argc, __argv_buf, __envp_buf);

  _exit (rc);
}
