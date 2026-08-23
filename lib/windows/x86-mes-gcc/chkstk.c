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

/* The stack probe a PE-targeting TinyCC calls for a large frame.
 *
 * i386-gen.c writes an ordinary prologue -- push %ebp, mov %esp,%ebp,
 * sub $size,%esp -- only while the frame is under 4096 bytes.  At or above
 * that it emits `mov $size,%eax; call __chkstk' instead and leaves the
 * whole prologue to this, because Windows grows a thread's stack by
 * touching one guard page at a time: a single sub past the guard page
 * lands in memory that was never committed and faults, so each page has to
 * be touched on the way down.
 *
 * TinyCC keeps this in win32/lib/chkstk.S, which the tinycc this bootstrap
 * builds does not carry (its lib/Makefile names chkstk.o, but the file was
 * never in the tree).  So it lives here, in the C library that side is
 * linked against anyway -- the instructions are upstream's, transcribed
 * rather than reconstructed, this being exactly the kind of assembly where
 * an off-by-one in the stack layout would corrupt silently.
 *
 * File-scope asm rather than a function: the contract is that __chkstk
 * builds the caller's frame itself and returns straight to the caller's
 * body, so it must not have a prologue of its own -- and it does not
 * return with `ret', which would find the caller's address under the frame
 * it just built, but jumps to the address it saved.
 *
 * On entry %eax holds the frame size and (%esp) the return address.  On
 * exit %ebp is the caller's frame pointer, the old %ebp is saved beneath
 * it, %esp is %ebp minus the frame size, and every page in between has
 * been touched.  %ecx is preserved; %eax is not, matching upstream.
 */

__asm__ (
".globl __chkstk\n"
"__chkstk:\n"
"  xchg %ebp,(%esp)\n"        /* save ebp, take the return address */
"  push %ebp\n"               /* put the return address back */
"  lea 4(%esp),%ebp\n"        /* frame pointer, as the short prologue sets */
"  push %ecx\n"
"  mov %ebp,%ecx\n"
"__chkstk_probe:\n"
"  sub $4096,%ecx\n"
"  test %eax,(%ecx)\n"        /* touch this page, so the guard page moves */
"  sub $4096,%eax\n"
"  cmp $4096,%eax\n"
"  jge __chkstk_probe\n"
"  sub %eax,%ecx\n"
"  test %eax,(%ecx)\n"        /* and the last, partial one */
"  mov %esp,%eax\n"
"  mov %ecx,%esp\n"           /* the frame is now allocated */
"  mov (%eax),%ecx\n"         /* restore ecx from where it was pushed */
"  jmp *4(%eax)\n"            /* and on to the caller's body */
);
