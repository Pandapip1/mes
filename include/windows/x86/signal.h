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

/* The machine state a signal handler is shown, which on Windows nothing ever
 * shows it.
 *
 * <signal.h> includes this for the types it names -- gregset_t, mcontext_t,
 * ucontext_t -- and lib/stub/sigaction.c and the rest are what a Windows
 * program gets when it asks for a signal, so nothing here is ever filled in.
 * The definitions are Linux's, unchanged, because the registers are the
 * machine's and the machine is the same one; the order they are stored in is
 * a Linux convention that costs nothing to keep and means the two kernels'
 * <signal.h> describe the same struct.
 */

#define EBX 0
#define ECX 1
#define EDX 2
#define ESI 3
#define EDI 4
#define EBP 5
#define EAX 6
#define DS 7
#define ES 8
#define FS 9
#define GS 10
#define ORIG_EAX 11
#define EIP 12
#define CS  13
#define EFL 14
#define UESP 15
#define SS   16
#define FRAME_SIZE 17

/* Type for general register.  */
typedef int greg_t;

/* Number of general registers.  */
#define NGREG        19

/* Container for all general registers.  */
typedef greg_t gregset_t[NGREG];

/* Definitions taken from the kernel headers.  */
struct _libc_fpreg
{
  unsigned short int significand[4];
  unsigned short int exponent;
};

struct _libc_fpstate
{
  unsigned long int cw;
  unsigned long int sw;
  unsigned long int tag;
  unsigned long int ipoff;
  unsigned long int cssel;
  unsigned long int dataoff;
  unsigned long int datasel;
  struct _libc_fpreg _st[8];
  unsigned long int status;
};

/* Structure to describe FPU registers.  */
typedef struct _libc_fpstate *fpregset_t;

typedef struct
{
  gregset_t gregs;
  /* Due to Linux's history we have to use a pointer here.  The SysV/i386
     ABI requires a struct with the values.  */
  fpregset_t fpregs;
  unsigned long int oldmask;
  unsigned long int cr2;
} mcontext_t;

/* Userlevel context.  */
typedef struct ucontext
{
  unsigned long int uc_flags;
  struct ucontext *uc_link;
  stack_t uc_stack;
  mcontext_t uc_mcontext;
  sigset_t uc_sigmask;
  struct _libc_fpstate __fpregs_mem;
} ucontext_t;
