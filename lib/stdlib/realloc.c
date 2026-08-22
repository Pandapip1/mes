/* -*-comment-start: "//";comment-end:""-*-
 * GNU Mes --- Maxwell Equations of Software
 * Copyright © 2016,2017,2018,2019 Jan (janneke) Nieuwenhuizen <janneke@gnu.org>
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

#include <stdlib.h>
#include <string.h>

/* lib/linux/malloc.c keeps one word before every pointer it returns,
 * holding that allocation's own real size -- read here to copy only
 * what the old block actually holds. Copying `size` (the new,
 * requested size, all this used to have to go on) instead reads past
 * the old block into whatever a *later* allocation already put there,
 * silently splicing unrelated live data into the grown copy. */
void *
realloc (void *ptr, size_t size)
{
  void *new = malloc (size);
  long *hdr;
  long old_size;
  size_t copy_size;

  if (ptr != 0 && new != 0)
    {
      hdr = ptr;
      hdr = hdr - 1;
      old_size = *hdr;
      copy_size = size;
      if (old_size < size)
        copy_size = old_size;
      memcpy (new, ptr, copy_size);
      free (ptr);
    }
  return new;
}
