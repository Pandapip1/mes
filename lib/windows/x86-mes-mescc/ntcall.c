/* -*-comment-start: "//";comment-end:""-*-
 * GNU Mes --- Maxwell Equations of Software
 * Copyright © 2026 Gavin John <gavinnjohn@gmail.com>
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

/* Calling ntdll from code MesCC compiled.
 *
 * ntdll is stdcall: the routine pops its own arguments before it returns.
 * MesCC compiles an indirect call as cdecl -- module/mescc/i386/as.scm's
 * i386:call-r emits `call *%r' and then `add $n*4,%esp', adding back what
 * it pushed.  Against a callee that already popped them, that add is a
 * second pop: the stack pointer ends up 4n bytes above where it belongs,
 * and every call walks it further until the frame it returns into is gone.
 *
 * M2-Planet has no such trouble, and lib/windows/x86-mes-m2/ntdll.c says
 * why: it saves the stack pointer before pushing the arguments and restores
 * it from there afterwards, rather than adding back.  That is correct for
 * either convention -- restoring a saved pointer costs nothing when the
 * callee popped, and undoes the pushes when it did not -- so this file does
 * the same thing for MesCC, in one place, rather than teaching MesCC's code
 * generator a second convention.  Nothing in MesCC changes; only these
 * functions know that ntdll pops.
 *
 * Each __ntcallN takes the routine and N arguments and calls it with them in
 * the order they are written.  The arguments go on the stack backwards --
 * the first argument nearest the stack pointer -- which is what a stdcall
 * callee expects, and it is done here so that no caller has to write its
 * arguments in reverse the way the M2 port does.
 *
 * %edi holds the saved stack pointer across the call.  It is pushed first
 * and popped last, so a caller that was using it keeps it, and ntdll is
 * required to preserve it as well.  Every argument is read straight from
 * this function's own frame, so nothing is computed between saving the
 * stack pointer and the call -- there is no argument here that could
 * clobber the register holding the routine's address, which is the third
 * thing the M2 port has to be careful about.
 *
 * GENERATED-LOOKING BUT HAND-MAINTAINED: the arities differ only in how
 * many arguments they push.  11 is the largest ntdll routine this C ever
 * calls (NtCreateFile).
 */

#include <windows/ntcall.h>

int
__ntcall0 (int fn)
{
  asm ("push___%edi");
  asm ("mov____%esp,%edi");
  asm ("mov____0x8(%ebp),%eax !8");
  asm ("call___*%eax");
  asm ("mov____%edi,%esp");
  asm ("pop____%edi");
}

int
__ntcall1 (int fn, int a1)
{
  asm ("push___%edi");
  asm ("mov____%esp,%edi");
  asm ("push___0x8(%ebp) !12");
  asm ("mov____0x8(%ebp),%eax !8");
  asm ("call___*%eax");
  asm ("mov____%edi,%esp");
  asm ("pop____%edi");
}

int
__ntcall2 (int fn, int a1, int a2)
{
  asm ("push___%edi");
  asm ("mov____%esp,%edi");
  asm ("push___0x8(%ebp) !16");
  asm ("push___0x8(%ebp) !12");
  asm ("mov____0x8(%ebp),%eax !8");
  asm ("call___*%eax");
  asm ("mov____%edi,%esp");
  asm ("pop____%edi");
}

int
__ntcall3 (int fn, int a1, int a2, int a3)
{
  asm ("push___%edi");
  asm ("mov____%esp,%edi");
  asm ("push___0x8(%ebp) !20");
  asm ("push___0x8(%ebp) !16");
  asm ("push___0x8(%ebp) !12");
  asm ("mov____0x8(%ebp),%eax !8");
  asm ("call___*%eax");
  asm ("mov____%edi,%esp");
  asm ("pop____%edi");
}

int
__ntcall4 (int fn, int a1, int a2, int a3, int a4)
{
  asm ("push___%edi");
  asm ("mov____%esp,%edi");
  asm ("push___0x8(%ebp) !24");
  asm ("push___0x8(%ebp) !20");
  asm ("push___0x8(%ebp) !16");
  asm ("push___0x8(%ebp) !12");
  asm ("mov____0x8(%ebp),%eax !8");
  asm ("call___*%eax");
  asm ("mov____%edi,%esp");
  asm ("pop____%edi");
}

int
__ntcall5 (int fn, int a1, int a2, int a3, int a4, int a5)
{
  asm ("push___%edi");
  asm ("mov____%esp,%edi");
  asm ("push___0x8(%ebp) !28");
  asm ("push___0x8(%ebp) !24");
  asm ("push___0x8(%ebp) !20");
  asm ("push___0x8(%ebp) !16");
  asm ("push___0x8(%ebp) !12");
  asm ("mov____0x8(%ebp),%eax !8");
  asm ("call___*%eax");
  asm ("mov____%edi,%esp");
  asm ("pop____%edi");
}

int
__ntcall6 (int fn, int a1, int a2, int a3, int a4, int a5, int a6)
{
  asm ("push___%edi");
  asm ("mov____%esp,%edi");
  asm ("push___0x8(%ebp) !32");
  asm ("push___0x8(%ebp) !28");
  asm ("push___0x8(%ebp) !24");
  asm ("push___0x8(%ebp) !20");
  asm ("push___0x8(%ebp) !16");
  asm ("push___0x8(%ebp) !12");
  asm ("mov____0x8(%ebp),%eax !8");
  asm ("call___*%eax");
  asm ("mov____%edi,%esp");
  asm ("pop____%edi");
}

int
__ntcall7 (int fn, int a1, int a2, int a3, int a4, int a5, int a6, int a7)
{
  asm ("push___%edi");
  asm ("mov____%esp,%edi");
  asm ("push___0x8(%ebp) !36");
  asm ("push___0x8(%ebp) !32");
  asm ("push___0x8(%ebp) !28");
  asm ("push___0x8(%ebp) !24");
  asm ("push___0x8(%ebp) !20");
  asm ("push___0x8(%ebp) !16");
  asm ("push___0x8(%ebp) !12");
  asm ("mov____0x8(%ebp),%eax !8");
  asm ("call___*%eax");
  asm ("mov____%edi,%esp");
  asm ("pop____%edi");
}

int
__ntcall8 (int fn, int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8)
{
  asm ("push___%edi");
  asm ("mov____%esp,%edi");
  asm ("push___0x8(%ebp) !40");
  asm ("push___0x8(%ebp) !36");
  asm ("push___0x8(%ebp) !32");
  asm ("push___0x8(%ebp) !28");
  asm ("push___0x8(%ebp) !24");
  asm ("push___0x8(%ebp) !20");
  asm ("push___0x8(%ebp) !16");
  asm ("push___0x8(%ebp) !12");
  asm ("mov____0x8(%ebp),%eax !8");
  asm ("call___*%eax");
  asm ("mov____%edi,%esp");
  asm ("pop____%edi");
}

int
__ntcall9 (int fn, int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8, int a9)
{
  asm ("push___%edi");
  asm ("mov____%esp,%edi");
  asm ("push___0x8(%ebp) !44");
  asm ("push___0x8(%ebp) !40");
  asm ("push___0x8(%ebp) !36");
  asm ("push___0x8(%ebp) !32");
  asm ("push___0x8(%ebp) !28");
  asm ("push___0x8(%ebp) !24");
  asm ("push___0x8(%ebp) !20");
  asm ("push___0x8(%ebp) !16");
  asm ("push___0x8(%ebp) !12");
  asm ("mov____0x8(%ebp),%eax !8");
  asm ("call___*%eax");
  asm ("mov____%edi,%esp");
  asm ("pop____%edi");
}

int
__ntcall10 (int fn, int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8, int a9, int a10)
{
  asm ("push___%edi");
  asm ("mov____%esp,%edi");
  asm ("push___0x8(%ebp) !48");
  asm ("push___0x8(%ebp) !44");
  asm ("push___0x8(%ebp) !40");
  asm ("push___0x8(%ebp) !36");
  asm ("push___0x8(%ebp) !32");
  asm ("push___0x8(%ebp) !28");
  asm ("push___0x8(%ebp) !24");
  asm ("push___0x8(%ebp) !20");
  asm ("push___0x8(%ebp) !16");
  asm ("push___0x8(%ebp) !12");
  asm ("mov____0x8(%ebp),%eax !8");
  asm ("call___*%eax");
  asm ("mov____%edi,%esp");
  asm ("pop____%edi");
}

int
__ntcall11 (int fn, int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8, int a9, int a10, int a11)
{
  asm ("push___%edi");
  asm ("mov____%esp,%edi");
  asm ("push___0x8(%ebp) !52");
  asm ("push___0x8(%ebp) !48");
  asm ("push___0x8(%ebp) !44");
  asm ("push___0x8(%ebp) !40");
  asm ("push___0x8(%ebp) !36");
  asm ("push___0x8(%ebp) !32");
  asm ("push___0x8(%ebp) !28");
  asm ("push___0x8(%ebp) !24");
  asm ("push___0x8(%ebp) !20");
  asm ("push___0x8(%ebp) !16");
  asm ("push___0x8(%ebp) !12");
  asm ("mov____0x8(%ebp),%eax !8");
  asm ("call___*%eax");
  asm ("mov____%edi,%esp");
  asm ("pop____%edi");
}
