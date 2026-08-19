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

/* Every ntdll call that touches a file reports what it actually transferred
 * in an IO_STATUS_BLOCK -- two words, the status and the count -- which the
 * caller has to provide.
 *
 * read and write cannot ask malloc for one.  They are compiled before
 * lib/linux/malloc.c in this build, as they are in the Linux one, and eputs
 * on the way to a fatal error has to work whether or not a heap was ever set
 * up.  So they use the one lib/m2/x86/ntdll-i386.hex2 already carries, which
 * is what its own reading and writing uses and which nothing else here
 * touches.  One block between them is enough because nothing is reentrant:
 * there is a single thread, and no read happens inside a write.
 */

int *
__iosb ()
{
  asm ("mov_eax, &iosb");
}
