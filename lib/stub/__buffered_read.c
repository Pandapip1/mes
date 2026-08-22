/* -*-comment-start: "//";comment-end:""-*-
 * GNU Mes --- Maxwell Equations of Software
 * Copyright © 2019,2022 Jan (janneke) Nieuwenhuizen <janneke@gnu.org>
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
#include <stdlib.h>
#include <string.h>

void
__buffered_read_init (int filedes)
{
}

/* Zero, and not merely falling off the end: both callers act on what this
 * says.  write and lseek ask how far a buffered read has already carried the
 * file position past where the caller believes it is, and seek back by that
 * much before doing anything -- so a value left in the return register by
 * whatever was called last becomes a seek to a place nothing meant, and every
 * write from then on lands on top of what came before it.  Where reads are
 * not buffered at all, which is the whole reason this stub stands in, the
 * honest answer is none. */
size_t
__buffered_read_clear (int filedes)
{
  return 0;
}
