/* -*-comment-start: "//";comment-end:""-*-
 * GNU Mes --- Maxwell Equations of Software
 * Copyright © 2016,2017,2018,2019,2021,2022 Jan (janneke) Nieuwenhuizen <janneke@gnu.org>
 * Copyright © 2021 Danny Milosavljevic <dannym@scratchpost.org>
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
#include <string.h>
#include <stddef.h>
#include <stdint.h>

char *__brk = 0;

/* A word, right before every pointer this returns, holding how many bytes
 * that allocation actually asked for. Nothing here ever frees -- brk only
 * moves forward -- so this never has to survive a reused block the way a
 * real allocator's bookkeeping would; it exists for exactly one reader:
 * realloc, which cannot otherwise know how much of the old block is real
 * data to preserve and how much is just whatever brk's own zero-filled,
 * never-yet-written memory beyond it happens to hold. Without this,
 * realloc's own memcpy (size, not the old allocation's size, is all it is
 * ever given) reads past the old block into memory some *later*
 * allocation already owns, copying that allocation's live data into the
 * grown copy as if it were the original's -- observed directly as tcc's
 * own object-file writer, whose growing Section buffers are exactly this
 * realloc pattern, silently splicing one section's string-table bytes
 * into another's, though it can happen anywhere something reallocs. */
void *
malloc (size_t size)
{
  long *hdr;
  char *p;

  if (!__brk)
    __brk = cast_long_to_charp (brk (0));
#if !__M2__
  /* align what we give back. */
  __brk = (char*) (((uintptr_t) __brk
                    + sizeof (max_align_t) - 1) & -sizeof (max_align_t));
#endif
  hdr = (long *) __brk;
  p = __brk + sizeof (long);
  if (brk (p + size) == -1)
    return 0;
  *hdr = size;
  __brk = p + size;
  return p;
}
