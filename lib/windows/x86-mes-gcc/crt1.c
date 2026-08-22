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

#define __ARGV_MAX 256
#define __ARG_BYTES_MAX 0x8000
#define __ENVP_MAX 1024
#define __ENV_BYTES_MAX 0x10000

char *__argv_buf[__ARGV_MAX];
char __arg_bytes[__ARG_BYTES_MAX];
char *__envp_buf[__ENVP_MAX];
char __env_bytes[__ENV_BYTES_MAX];

/* One argument off the command line at *cursor, narrowed and unescaped into
 * *out_cursor, which this advances past the token and its NUL the way
 * *cursor is advanced past the token and its separator. Returns 0 (cursor
 * unmoved past trailing whitespace) when the command line has no more
 * tokens left. */
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
                  *o++ = '\\';
                  i = i + 1;
                }
              if (backslashes % 2)
                {
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

      *o++ = *p;
      p += 2;
    }

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
void
__init_env (char *env)
{
  int n;
  char *o;
  char c;

  n = 0;
  o = __env_bytes;
  while (*env != 0 && n < __ENVP_MAX - 1)
    {
      __envp_buf[n] = o;
      for (;;)
        {
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

  peb = __peb ();
  pp = *(int *) (peb + 16);            /* PEB->ProcessParameters */
  cmdline = *(char **) (pp + 68);      /* ->CommandLine.Buffer */
  envblock = *(char **) (pp + 72);     /* ->Environment */

  cursor = cmdline;
  out = __arg_bytes;
  argc = 0;
  while (argc < __ARGV_MAX - 1)
    {
      tok = out;
      if (!__next_arg (&cursor, &out))
        break;
      __argv_buf[argc] = tok;
      argc = argc + 1;
    }
  __argv_buf[argc] = 0;

  __init_env (envblock);

  __init_io (argc, __argv_buf, __envp_buf);

  rc = main (argc, __argv_buf, __envp_buf);

  _exit (rc);
}
