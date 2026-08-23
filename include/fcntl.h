/* -*-comment-start: "//";comment-end:""-*-
 * GNU Mes --- Maxwell Equations of Software
 * Copyright © 2017 Jan (janneke) Nieuwenhuizen <janneke@gnu.org>
 * Copyright © 2021 W. J. van der Laan <laanwj@protonmail.com>
 * Copyright © 2023 Emily Trau <emily@downunderctf.com>
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
#ifndef __MES_FCNTL_H
#define __MES_FCNTL_H 1

#if SYSTEM_LIBC
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#undef __MES_FCNTL_H
#include_next <fcntl.h>

#else // ! SYSTEM_LIBC

// *INDENT-OFF*
/* Windows shares Linux's numbers here because nothing on Windows consults
 * them directly: lib/windows/_open3.c reads the flags and works out the
 * DesiredAccess and CreateDisposition NtCreateFile actually wants.  Picking
 * the same constants means every caller above is unchanged.
 *
 * Three names for the one target because three things build this.  M2-Planet
 * and MesCC are told __windows__ on the command line, that being what the
 * rest of the PE32 bootstrap passes.  A TinyCC configured for PE says _WIN32
 * of its own accord, and says __linux__ only when it is not -- so the round
 * where tcc starts recompiling this library for Windows arrives with neither
 * of the first two defined, and stopped here at "platform not supported".
 * This is the only line in the tree that asks which system it is built for. */
#if __linux__ || __windows__ || _WIN32
#define O_RDONLY          0
#define O_WRONLY          1
#define O_RDWR            2
#define O_CREAT        0x40
#define O_EXCL         0x80
#define O_TRUNC       0x200
#define O_APPEND      0x400

#ifdef __arm__
#define O_DIRECTORY   0x4000
#define O_TMPFILE   0x404000
#else
#define O_DIRECTORY  0x10000
#define O_TMPFILE   0x410000
#endif

#define AT_FDCWD            -100
#define AT_SYMLINK_NOFOLLOW  0x100
#define AT_REMOVEDIR         0x200

#elif __GNU__
#define	O_RDONLY	  1
#define	O_WRONLY	  2
#define	O_RDWR		  3
#define	O_CREAT	       0x10
#define	O_APPEND      0x100
#define	O_TRUNC	    0x10000
#else
#error platform not supported
#endif
// *INDENT-ON*

#define FD_CLOEXEC 1

#define F_DUPFD 0
#define F_GETFD 1
#define F_SETFD 2
#define F_GETFL 3
#define F_SETFL 4

#define creat(file_name, mode) open (file_name, O_WRONLY | O_CREAT | O_TRUNC, mode)
int dup (int old);
int dup2 (int old, int new);

#if !__M2__
int fcntl (int filedes, int command, ...);
int open (char const *s, int flags, ...);
#endif

#endif // ! SYSTEM_LIBC

#endif // __MES_FCNTL_H
