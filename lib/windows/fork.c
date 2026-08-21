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
#include <windows/x86-mes-m2/ntdll.h>
#include <mes/lib.h>

/* Stands in for lib/linux/fork.c, and now works on 64-bit Windows under
 * WOW64 -- one real OS-level address-space clone, no second process spawned
 * from scratch.  Three things this file used to say are worth correcting
 * rather than leaving, and are kept below since the mistakes they made are
 * exactly the ones worth not making again:
 *
 *   RtlCloneUserProcess does not wrap NtCreateProcessEx.  Disassembling
 *   ntdll32.dll shows it reaching ZwCreateUserProcess, the same system call
 *   behind ordinary process creation.  They are two unrelated ways into the
 *   kernel, so a defect found in one says nothing about the other.
 *
 *   The flag combinations are not all alike.  Without NO_SYNCHRONIZE the
 *   child deadlocks; with it the child runs and dies of an access violation.
 *   And the deadlock turned out to be downstream of the fault rather than a
 *   second failure beside it: what waits on the inherited lock is the
 *   exception dispatcher, trying to report the access violation.
 *
 *   It is not "neither this port nor WOW64".  That was drawn from the same
 *   call failing from PowerShell too, where the child was stopped by the
 *   inherited lock -- a different failure.  Where the clone's child actually
 *   stops is `mov eax, fs:0x18', faulting on linear address 0x18: the FS
 *   selector is the right one and the descriptor behind it has no base.
 *
 * All three of those were stage0-pe32's own findings, in
 * x86/M2libc-windows/process.c's __clone_process, and none of them turned out
 * to be a wall.  wow64cpu!RunSimulatedCode -- the loop that runs 32-bit code
 * at all -- does not take r13/r15 as arguments; it loads them itself, from
 * the thread's own TEB and CPU area, on every entry.  So a cloned thread's
 * initial context can be pointed straight at RunSimulatedCode's own exported
 * entry, BTCpuSimulate, from user mode: a native NtSetContextThread call
 * (wow64gate.c's gate, since the ordinary WOW64-thunked one only reaches the
 * 32-bit CPU-area CONTEXT) plus the ordinary NtSetContextThread this port
 * already uses elsewhere, to force Eax to STATUS_PROCESS_CLONED.  That alone
 * still deadlocked, on an internal ntdll lock RtlCloneUserProcess holds
 * across its own clone syscall and never releases on the child's branch;
 * zeroing that lock's four bytes in the child before resuming is the rest of
 * the fix.  stage0-pe32 fcc842d ("x86: the WOW64 clone runs from user mode,
 * and fork uses it") is where all of that was found and measured, and
 * x86/Development/wow64-clone-driver.md has the disassembly and every
 * measurement in full.  __clone_process_wow64fix below is the same fix,
 * moved into this project's own C.
 *
 * What an earlier version of this file said about copying instead is still
 * right, and still the reason that road stays closed here specifically:
 * stage0-pe32 also has a working fork built by starting the same program
 * again and overwriting the copy with this one, which works there because
 * its image is at a fixed address, with code, globals, heap and even the
 * kernel's own stack laid out identically in every process that runs it.
 * Mes does not get that for free, and lib/windows/brk.c is why: this
 * project's heap lives outside its image, at a base the kernel chooses and a
 * size discovered by halving a reservation, not at a fixed offset a second
 * process would already agree with.  But that objection is about the
 * copy-based fork specifically, not about cloning in general -- a real
 * OS-level address-space clone has no such mismatch to work around, because
 * the whole address space, wherever it lives, gets cloned as-is, heap base
 * included.  So only the copy-based half of the old comment still applies,
 * and only the copy-based fork is left unported: not attempted here, and not
 * missed, since __clone_process_wow64fix does not need it.
 *
 * Until a real 32-bit Windows or Wine measurement says otherwise, fork falls
 * back to plain -1 whenever the clone-based fix cannot run at all: no
 * RtlCloneUserProcess (absent on Wine, where it resolves to 0), or a clone
 * that fails for some other reason.  What Windows can do instead is start a
 * program directly, which is lib/windows/execve.c, and wait for it, which is
 * lib/windows/waitpid.c:
 *
 *   pid = __spawn (file, argv, envp); waitpid (pid, &status, 0);
 *
 * rather than fork, execve in the child and waitpid in the parent.
 */

/* The same RtlCloneUserProcess __clone_process_wow64fix below uses, without
 * the WOW64 fix applied -- what fork would already be if the clone's child
 * came back on its own.  Kept for two reasons: it is __clone_process_wow64fix's
 * own fallback when this process turns out not to be running under WOW64 at
 * all (see there), and a system where the plain clone worked would get a
 * cheaper fork than the fix below for free.  stage0-pe32's own
 * __clone_process, in x86/M2libc-windows/process.c, carries the measurements
 * this is built from -- the fs:0x18 fault a plain clone's child dies of under
 * WOW64, worked out in full there -- and is not repeated here.  Not tested on
 * genuine 32-bit Windows or on Wine; reasoned through, per stage0-pe32's own
 * note, rather than measured. */
int
__clone_process ()
{
  int (*RtlCloneUserProcess) (int, int, int, int, int);
  int (*NtResumeThread) (int, int);
  int *info;
  int thread;
  int i;
  int rc;

  RtlCloneUserProcess = __ntdll_resolve ("RtlCloneUserProcess");
  if (RtlCloneUserProcess == 0)
    return -1;

  /* RTL_USER_PROCESS_INFORMATION, as execve.c's __spawn fills in. */
  info = malloc (128);
  i = 0;
  while (i < 32)
    {
      info[i] = 0;
      i = i + 1;
    }
  info[0] = 68;

  /* forwards: RtlCloneUserProcess (RTL_CLONE_PROCESS_FLAGS_INHERIT_HANDLES,
   *                                0, 0, 0, info) */
  rc = RtlCloneUserProcess (info, 0, 0, 0, 2);

  if (rc == 0x129)
    return 0;                   /* STATUS_PROCESS_CLONED: this is the child */
  if (rc != 0)
    return -1;

  /* The clone's first thread starts suspended, the same way __spawn's does,
   * so the child does not come back from the call above until let go. */
  NtResumeThread = __ntdll_resolve ("NtResumeThread");
  thread = info[2];
  /* forwards: NtResumeThread (thread, 0) */
  NtResumeThread (0, thread);

  return info[1];               /* the parent gets a handle to it */
}

/* __clone_process_wow64fix: the same RtlCloneUserProcess as __clone_process
 * above, with stage0-pe32 fcc842d's fix applied before the child is ever let
 * run.  See this file's own head comment, wow64gate.c and wow64resolve.c for
 * the mechanism, and x86/Development/wow64-clone-driver.md in stage0-pe32
 * for the disassembly and the measurements this is built from -- none of
 * that is repeated here, only the steps that carry it out.
 *
 * CONTEXT_AMD64 byte offsets used below (ContextFlags +0x30, SegCs +0x38,
 * SegSs +0x42, EFlags +0x44, Rsp +0x98, Rip +0xF8) are mingw-w64's winnt.h
 * struct _CONTEXT (x86_64), cross-checked in wow64-clone-driver.md sec
 * 9.2/9.3 against a live read on stage0-pe32's win11 VM. */
int
__clone_process_wow64fix ()
{
  int (*RtlCloneUserProcess) (int, int, int, int, int);
  int (*NtResumeThread) (int, int);
  int (*NtGetContextThread) (int, int);
  int (*NtSetContextThread) (int, int);
  int (*NtTerminateProcess) (int, int);
  int (*NtWriteVirtualMemory) (int, int, int, int, int);
  int *info;
  int gate;
  int *peb_hi;
  int peb_lo;
  int ntdll64_lo;
  int *ntdll64_hi;
  int wow64cpu_lo;
  int *wow64cpu_hi;
  int getctx_lo;
  int *getctx_hi;
  int setctx_lo;
  int *setctx_hi;
  int btcpu_lo;
  int *btcpu_hi;
  int thread;
  int *ctxA;
  int *ctxB;
  int rspA_lo;
  int rspA_hi;
  int rtlclone_rva;
  int lock_rva;
  int lock_addr;
  int *zero;
  int *wrote;
  int child;
  int rc;
  int i;

  RtlCloneUserProcess = __ntdll_resolve ("RtlCloneUserProcess");
  if (RtlCloneUserProcess == 0)
    return -1;

  /* -- resolve everything the fix needs before touching the clone at all -- */
  peb_hi = malloc (4);
  peb_lo = __wow64_selfpeb (peb_hi);
  if (peb_lo == 0 && peb_hi[0] == 0)
    return -1;

  /* Everything from here down exists only because of WOW64: on genuine
   * 32-bit Windows there is no 64-bit kernel underneath, no simulator to
   * enter, no compat-mode FS-base indirection to miss, so this whole defect
   * class has nowhere to come from.  wow64cpu.dll not being findable in this
   * process's own 64-bit module list is the signal for that -- the same
   * shape RtlCloneUserProcess itself already uses for Wine, where it just
   * resolves to 0 rather than the caller being made to guess.  Fall back to
   * the plain clone: no gate, no 64-bit resolution, none of it.  Not tested
   * against real 32-bit Windows -- neither this project nor stage0-pe32 has
   * such hardware or VM to test on. */
  ntdll64_hi = malloc (4);
  ntdll64_lo = __wow64_find_module (peb_lo, peb_hi[0], "ntdll.dll",
                                     ntdll64_hi);
  wow64cpu_hi = malloc (4);
  wow64cpu_lo = __wow64_find_module (peb_lo, peb_hi[0], "wow64cpu.dll",
                                      wow64cpu_hi);
  if (wow64cpu_lo == 0 && wow64cpu_hi[0] == 0)
    return __clone_process ();
  if (ntdll64_lo == 0 && ntdll64_hi[0] == 0)
    return -1;

  getctx_hi = malloc (4);
  getctx_lo = __wow64_resolve_export (ntdll64_lo, ntdll64_hi[0],
                                       "NtGetContextThread", getctx_hi);
  setctx_hi = malloc (4);
  setctx_lo = __wow64_resolve_export (ntdll64_lo, ntdll64_hi[0],
                                       "NtSetContextThread", setctx_hi);
  btcpu_hi = malloc (4);
  btcpu_lo = __wow64_resolve_export (wow64cpu_lo, wow64cpu_hi[0],
                                      "BTCpuSimulate", btcpu_hi);
  if (getctx_lo == 0 && getctx_hi[0] == 0)
    return -1;
  if (setctx_lo == 0 && setctx_hi[0] == 0)
    return -1;
  if (btcpu_lo == 0 && btcpu_hi[0] == 0)
    return -1;

  gate = __gate_init ();
  if (gate == 0)
    return -1;

  /* -- clone, suspended -- */
  info = malloc (128);
  i = 0;
  while (i < 32)
    {
      info[i] = 0;
      i = i + 1;
    }
  info[0] = 68;
  /* forwards: RtlCloneUserProcess (RTL_CLONE_PROCESS_FLAGS_CREATE_SUSPENDED |
   *                                RTL_CLONE_PROCESS_FLAGS_INHERIT_HANDLES,
   *                                0, 0, 0, info) -- 3, not bare
   * CREATE_SUSPENDED (1).  stage0-pe32 4be4514 found this the same way: with
   * flags=1 alone the child silently loses its inherited handles -- stdout,
   * any open file -- rather than faulting, so nothing in this port's own
   * testing so far would have caught it either.  Ported here before that
   * gap could sit in this file too. */
  rc = RtlCloneUserProcess (info, 0, 0, 0, 3);
  if (rc == 0x129)
    return 0;                   /* the clone ran with no fix at all -- never
                                  * observed on stage0-pe32's VM, kept as a
                                  * cheap check in case a future Windows build
                                  * no longer needs any of this. */
  if (rc != 0)
    return -1;
  thread = info[2];
  child = info[1];

  /* -- Step A: native NtGetContextThread on the never-run initial thread -- */
  ctxA = __walloc16 (0x4D0);
  i = 0;
  while (i < 0x4D0 / 4)
    {
      ctxA[i] = 0;
      i = i + 1;
    }
  ctxA[12] = 0x100007;          /* ContextFlags at +0x30: CONTROL|INTEGER|SEGMENTS */
  rc = __gate_call (gate, getctx_lo, getctx_hi[0], thread, 0, ctxA, 0);
  if (rc != 0)
    {
      NtTerminateProcess = __ntdll_resolve ("NtTerminateProcess");
      NtTerminateProcess (1, child);
      return -1;
    }
  rspA_lo = ctxA[0x98 / 4];
  rspA_hi = ctxA[0x98 / 4 + 1];

  /* -- Step B: native NtSetContextThread, redirecting Rip -> BTCpuSimulate -- */
  ctxB = __walloc16 (0x4D0);
  i = 0;
  while (i < 0x4D0 / 4)
    {
      ctxB[i] = 0;
      i = i + 1;
    }
  ctxB[12] = 0x100001;          /* CTXF_CONTROL: Rip/Rsp/SegCs/SegSs/EFlags */
  ctxB[0xF8 / 4] = btcpu_lo;
  ctxB[0xF8 / 4 + 1] = btcpu_hi[0];       /* Rip */
  ctxB[0x98 / 4] = rspA_lo;
  ctxB[0x98 / 4 + 1] = rspA_hi;           /* Rsp: the thread's own */
  i = ctxB[0x38 / 4];
  ctxB[0x38 / 4] = (i & -65536) | 0x33;   /* SegCs, low 16 bits */
  i = ctxB[0x40 / 4];
  ctxB[0x40 / 4] = (i & 65535) | 0x2B * 65536;   /* SegSs at +0x42 */
  ctxB[0x44 / 4] = 0x202;        /* EFlags */
  rc = __gate_call (gate, setctx_lo, setctx_hi[0], thread, 0, ctxB, 0);
  if (rc != 0)
    {
      NtTerminateProcess = __ntdll_resolve ("NtTerminateProcess");
      NtTerminateProcess (1, child);
      return -1;
    }

  /* -- Step C: the ordinary (wow64-thunked) NtSetContextThread this port
   * already uses elsewhere, on the 32-bit CPU-area CONTEXT, asking only for
   * CONTEXT_INTEGER|CONTEXT_CONTROL|CONTEXT_SEGMENTS (0x10007) with Eax
   * forced to STATUS_PROCESS_CLONED.  wow64-clone-driver.md sec 8.2's
   * disassembly of BTCpuSetContext is why this is enough: asking for that
   * flag set is what makes it OR bit 0 into the CPU area's status word, the
   * same bit RunSimulatedCode's own entry tests to decide whether to
   * reprogram FS.  Get first, so nothing already correct in the CPU-area
   * CONTEXT is stepped on -- only Eax changes. */
  NtGetContextThread = __ntdll_resolve ("NtGetContextThread");
  NtSetContextThread = __ntdll_resolve ("NtSetContextThread");
  ctxA = __walloc16 (0x2CC);
  i = 0;
  while (i < 179)
    {
      ctxA[i] = 0;
      i = i + 1;
    }
  ctxA[0] = 0x10007;
  rc = NtGetContextThread (ctxA, thread);
  if (rc != 0)
    {
      NtTerminateProcess = __ntdll_resolve ("NtTerminateProcess");
      NtTerminateProcess (1, child);
      return -1;
    }
  ctxA[44] = 0x129;              /* Eax at +0xB0 */
  rc = NtSetContextThread (ctxA, thread);
  if (rc != 0)
    {
      NtTerminateProcess = __ntdll_resolve ("NtTerminateProcess");
      NtTerminateProcess (1, child);
      return -1;
    }

  /* -- Step C.5: zero RtlCloneUserProcess's own internal SRW lock, in the
   * CHILD's copy, before it ever runs an instruction.  The parent's own call
   * (CREATE_SUSPENDED, no NO_SYNCHRONIZE) takes a branch that acquires two
   * SRW locks and holds both across the clone syscall inside it; the
   * child's branch releases one of them unconditionally but never the
   * other, ntdll+0x12d52c, before reaching an acquire-then-release call on
   * it a few instructions later -- a wait on a lock nothing in this process
   * will ever Release.  wow64-clone-driver.md sec 10's CFG-aware
   * disassembly (radare2 agf, not a linear read) is where this was found;
   * it is not repeated here.
   *
   * ntdll.dll carries no relocations for this component, so the parent's
   * own already-resolved RtlCloneUserProcess pointer and this process's own
   * copy of ntdll.dll sit at the same offset from each other as the
   * child's do; RtlCloneUserProcess's RVA (0xbaa60) and the lock's RVA
   * (0x12d52c) get from one to the other without ever naming ntdll's base
   * directly.  child is the child's process handle
   * (RTL_USER_PROCESS_INFORMATION.Process). */
  rtlclone_rva = 0xbaa60;
  lock_rva = 0x12d52c;
  lock_addr = RtlCloneUserProcess - rtlclone_rva + lock_rva;
  zero = malloc (4);
  zero[0] = 0;
  wrote = malloc (4);
  wrote[0] = 0;
  NtWriteVirtualMemory = __ntdll_resolve ("NtWriteVirtualMemory");
  /* forwards: NtWriteVirtualMemory (child, lock_addr, zero, 4, wrote) */
  rc = NtWriteVirtualMemory (wrote, 4, zero, lock_addr, child);
  if (rc != 0)
    {
      NtTerminateProcess = __ntdll_resolve ("NtTerminateProcess");
      NtTerminateProcess (1, child);
      return -1;
    }

  /* -- Step D: let it go -- */
  NtResumeThread = __ntdll_resolve ("NtResumeThread");
  /* forwards: NtResumeThread (thread, 0) */
  NtResumeThread (0, thread);

  return child;
}

/* fork -- see this file's own head comment for what it prefers and why, and
 * for what it falls back to and why that fallback is not the copy-based one
 * stage0-pe32 uses instead. */
int
fork ()
{
  return __clone_process_wow64fix ();
}
