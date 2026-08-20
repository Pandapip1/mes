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

/* The 32-to-64-to-32 heaven's-gate trampoline lib/windows/fork.c's
 * __clone_process_wow64fix needs, and the 64-bit arithmetic that goes with
 * it.  Ported from stage0-pe32 fcc842d, x86/M2libc-windows/process.c
 * (__qadd64, __gate_init, __gate_invoke, __gate_call); its own comment there,
 * and x86/Development/wow64-clone-driver.md sec 8-9, have the disassembly and
 * the measurements this is built from.  Only the mechanism moved; nothing
 * about why it works is repeated here.
 *
 * The gate exists because a native (64-bit) NtGetContextThread/
 * NtSetContextThread call is the only way to point a cloned thread's real,
 * native instruction pointer at wow64cpu!BTCpuSimulate, and this 32-bit
 * process cannot make that call the ordinary way: its own NtGetContextThread/
 * NtSetContextThread are thunked down to the 32-bit CPU-area CONTEXT, which
 * is a different, narrower thing.  A small 64-bit payload, sitting in a page
 * this process allocates PAGE_EXECUTE_READWRITE, does the native call
 * directly; a 32-bit stub at the front and back of the page carries the CPU
 * across the mode switch and back.  The payload's own bytes are exactly
 * stage0-pe32's -- machine code, not something the language dialect changes
 * moving from one project's C to another's.
 */

/* One 8-byte quantity, as (lo, hi).  Adding a small positive RVA to a 64-bit
 * base can carry into the high word even though every base and RVA here is
 * itself a small positive number, so the carry is worked out from the bit
 * pattern rather than assumed away: newlo = lo + delta wraps in two's
 * complement exactly the way an unsigned add would, and XORing both sides
 * with the sign bit before comparing turns that wrapped unsigned comparison
 * into one this dialect's signed `<' gets right, because XORing the sign bit
 * is a monotonic reordering from unsigned into signed.  hi_out is a one-word
 * scratch buffer the caller owns. */
int
__qadd64 (int lo, int hi, int delta, int *hi_out)
{
  int newlo;
  int carry;

  newlo = lo + delta;
  carry = 0;
  if ((newlo ^ -2147483648) < (lo ^ -2147483648))
    carry = 1;
  hi_out[0] = hi + carry;
  return newlo;
}

/* Mes's malloc gives back memory with no alignment guarantee at all under
 * M2-Planet -- lib/linux/malloc.c's own rounding-up is guarded `#if
 * !__M2__' and simply does not run in this build -- which every 64-bit Nt*
 * call in wow64resolve.c and every CONTEXT this file's callers pass across
 * the gate both need: PROCESS_BASIC_INFORMATION64 and CONTEXT_AMD64 (and, on
 * this same machine, the ordinary 32-bit CONTEXT) all hold naturally-aligned
 * 64-bit fields, and Windows answers STATUS_DATATYPE_MISALIGNMENT rather
 * than fix up an unaligned one -- measured directly, porting this file,
 * where an unaligned PROCESS_BASIC_INFORMATION64 buffer is exactly what
 * turned into that status.  16 bytes of slack past the requested size is
 * always enough room to round up to a 16-byte boundary and still have
 * `size' usable bytes left, whether the caller needs 8-byte alignment or
 * (as stage0-pe32's own CONTEXT buffer already does) 16. */
int *
__walloc16 (int size)
{
  int p;

  p = malloc (size + 16);
  p = p + 15;
  p = p - (p % 16);
  return p;
}

/* The gate itself: a fixed 64-bit payload (x86_64-linux-gnu-as + objcopy -O
 * binary of stage0-pe32's x86/Development/gate.S, bytes identical to that
 * project's own build) in a page this process allocates
 * PAGE_EXECUTE_READWRITE.  Layout (byte offsets): 0x000 a 32-bit entry stub;
 * 0x040 a data area of six 8-byte slots (Target, Arg1..Arg4, Result); 0x0C0
 * the 64-bit payload; 0x120 a 32-bit return stub.  Only the 89 non-zero bytes
 * out of 289 are written here; the rest of the fresh page is already zero.
 *
 * Cached in a global, the same way __wow64_selfhandle in wow64resolve.c
 * caches its own handle: the bytes below never change from one call to the
 * next, so a second call has nothing to do differently.  Before this, every
 * fork () allocated a fresh PAGE_EXECUTE_READWRITE page here and never freed
 * or reused it -- 4KB of committed, executable memory, leaked, once per
 * fork ().  stage0-pe32 5d32496 found and fixed the identical leak in its
 * own __gate_init, the function this was ported from; same fix here. */
int __gate_have;
int __gate_cache;
int
__gate_init ()
{
  int (*NtAllocateVirtualMemory) (int, int, int, int, int, int);
  char *g;
  int *base;
  int *size;
  int rc;

  if (__gate_have != 0)
    return __gate_cache;

  base = malloc (4);
  base[0] = 0;
  size = malloc (4);
  size[0] = 0x1000;

  NtAllocateVirtualMemory = __ntdll (NT_ALLOC);
  /* forwards: NtAllocateVirtualMemory (-1, base, 0, size, MEM_COMMIT|
   *   MEM_RESERVE, PAGE_EXECUTE_READWRITE) -- -1 is NtCurrentProcess: the
   *   gate lives in this same process, not the clone's. */
  rc = NtAllocateVirtualMemory (0x40, 0x3000, size, 0, base, -1);
  if (rc != 0)
    return 0;

  g = base[0];
  g[0x0] = 106; g[0x1] = 51; g[0x2] = 80; g[0x3] = 232;
  g[0x8] = 88; g[0x9] = 5; g[0xa] = 184; g[0xe] = 135;
  g[0xf] = 4; g[0x10] = 36; g[0x11] = 255; g[0x12] = 44;
  g[0x13] = 36; g[0xc0] = 72; g[0xc1] = 131; g[0xc2] = 196;
  g[0xc3] = 8; g[0xc4] = 72; g[0xc5] = 141; g[0xc6] = 29;
  g[0xc7] = 117; g[0xc8] = 255; g[0xc9] = 255; g[0xca] = 255;
  g[0xcb] = 72; g[0xcc] = 139; g[0xcd] = 75; g[0xce] = 8;
  g[0xcf] = 72; g[0xd0] = 139; g[0xd1] = 83; g[0xd2] = 16;
  g[0xd3] = 76; g[0xd4] = 139; g[0xd5] = 67; g[0xd6] = 24;
  g[0xd7] = 76; g[0xd8] = 139; g[0xd9] = 75; g[0xda] = 32;
  g[0xdb] = 72; g[0xdc] = 137; g[0xdd] = 224; g[0xde] = 72;
  g[0xdf] = 131; g[0xe0] = 228; g[0xe1] = 240; g[0xe2] = 72;
  g[0xe3] = 131; g[0xe4] = 236; g[0xe5] = 64; g[0xe6] = 72;
  g[0xe7] = 137; g[0xe8] = 68; g[0xe9] = 36; g[0xea] = 56;
  g[0xeb] = 72; g[0xec] = 139; g[0xed] = 67; g[0xee] = 40;
  g[0xef] = 72; g[0xf0] = 137; g[0xf1] = 68; g[0xf2] = 36;
  g[0xf3] = 32; g[0xf4] = 72; g[0xf5] = 139; g[0xf6] = 3;
  g[0xf7] = 255; g[0xf8] = 208; g[0xf9] = 72; g[0xfa] = 139;
  g[0xfb] = 100; g[0xfc] = 36; g[0xfd] = 56; g[0xfe] = 72;
  g[0xff] = 137; g[0x100] = 67; g[0x101] = 48; g[0x102] = 72;
  g[0x103] = 141; g[0x104] = 5; g[0x105] = 23; g[0x109] = 106;
  g[0x10a] = 35; g[0x10b] = 80; g[0x10c] = 72; g[0x10d] = 203;
  g[0x120] = 195;

  __gate_cache = base[0];
  __gate_have = 1;
  return base[0];
}

/* Cast gate to a cdecl void(void) and call it.  Split out of __gate_call
 * only because this dialect's function-pointer declarations name their
 * argument count, and the gate genuinely takes none -- everything it needs
 * is already sitting in its own data area. */
int
__gate_invoke (int gate)
{
  int (*entry) ();

  entry = gate;
  entry ();
  return 0;
}

/* One call across the gate: target (a1, a2), both args and the target
 * zero-extended to 64 bits (every target and argument this file passes is
 * really 32 bits wide -- a HANDLE or a heap pointer of this process's own, or
 * a VA already carried as (lo, hi) -- so hi halves are written 0 or the
 * caller's own hi word directly).  Returns the low 32 bits of the 64-bit
 * result, which is all any caller here reads back. */
int
__gate_call (int gate, int target_lo, int target_hi, int a1, int a1_hi,
             int a2, int a2_hi)
{
  int *data;

  data = gate + 0x40;
  data[0] = target_lo; data[1] = target_hi;
  data[2] = a1; data[3] = a1_hi;
  data[4] = a2; data[5] = a2_hi;
  data[12] = -572662307;        /* 0xDCDCDCDC: a recognisable poison result,
                                  * so a gate call that never touches Result
                                  * reads back as obviously wrong rather than
                                  * as a plausible STATUS_SUCCESS of 0. */

  __gate_invoke (gate);

  return data[12];
}
