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
 * up.  The M2 port borrows the block lib/m2/x86/ntdll-i386.hex2 carries for
 * its own reading and writing; nothing hand-assembled runs before a MesCC
 * build, so there is no such block to borrow and this one is simply a
 * variable.  One block serves both because nothing is reentrant: there is a
 * single thread, and no read happens inside a write.
 */

int __iosb_block[2];

int *
__iosb ()
{
  return __iosb_block;
}
