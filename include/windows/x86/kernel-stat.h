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

/* Stands in for include/linux/x86/kernel-stat.h, which lays out the struct
 * stat that a Linux kernel fills in.
 *
 * There is deliberately no struct here.  Windows has no stat in this port:
 * lib/windows/ has no stat.c or fstat.c, and neither does the M2libc fork's
 * x86/windows/sys/stat.c, which carries chmod, fchmod and mkdir and nothing
 * that returns a struct.  Declaring a layout would be describing a shape
 * nothing on this side ever fills.
 *
 * sys/stat.h still includes this unconditionally, and its declarations of
 * stat and fstat take a struct stat * -- a pointer to an incomplete type,
 * which is a complete declaration in C.  A caller that tried to make one, or
 * to reach through it, would not compile, which is the correct outcome until
 * something here can fill it.
 */

#ifndef __MES_WINDOWS_KERNEL_STAT_H
#define __MES_WINDOWS_KERNEL_STAT_H 1

struct stat;

#endif /* __MES_WINDOWS_KERNEL_STAT_H */
