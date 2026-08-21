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

/* Reading this process's own 64-bit view of itself: its 64-bit PEB, the
 * 64-bit ntdll.dll and wow64cpu.dll images that share its address space
 * above 4GB, and the export tables inside them.  lib/windows/fork.c's
 * __clone_process_wow64fix needs the native VAs of NtGetContextThread,
 * NtSetContextThread and BTCpuSimulate, none of which a 32-bit pointer can
 * name, and this is how it gets them.
 *
 * Ported from stage0-pe32 fcc842d, x86/M2libc-windows/process.c
 * (__wow64_selfhandle, __wow64_rd64 and its dword/word helpers,
 * __wow64_selfpeb, __wow64_find_module, __wow64_resolve_export); see that
 * file's own comments and x86/Development/wow64-clone-driver.md sec 8-9 for
 * why each offset and each two-call resolution path is what it is.  Every
 * 64-bit address here is carried as two ints (*_lo, *_hi) -- this dialect has
 * no 64-bit int -- and __qadd64, in wow64gate.c, does the carry.
 */

/* A real (non-pseudo) handle to this process.  NtWow64ReadVirtualMemory64
 * and NtWow64QueryInformationProcess64 both answer STATUS_INVALID_HANDLE to
 * the NtCurrentProcess pseudo-handle (-1) every other Nt* call in this port
 * accepts without complaint -- measured in stage0-pe32's own port of this
 * same code.  NtDuplicateObject, given -1 as both the source and the target
 * process and -1 as the source handle, hands back a genuine handle meaning
 * the same process; lib/windows/dup.c and execve.c's __inheritable use the
 * same call for a different handle. */
int __wow64_self_handle_cache;
int __wow64_self_handle_have;

int
__wow64_selfhandle ()
{
  int (*NtDuplicateObject) (int, int, int, int, int, int, int);
  int *out;

  if (__wow64_self_handle_have != 0)
    return __wow64_self_handle_cache;

  out = malloc (4);
  out[0] = 0;
  NtDuplicateObject = __ntdll_resolve ("NtDuplicateObject");
  /* forwards: NtDuplicateObject (-1, -1, -1, out, 0, 0,
   *                              DUPLICATE_SAME_ACCESS) */
  if (NtDuplicateObject (2, 0, 0, out, -1, -1, -1) != 0)
    return -1;
  __wow64_self_handle_cache = out[0];
  __wow64_self_handle_have = 1;
  return out[0];
}

/* __wow64_rd64_got / __wow64_rd64_v: the two buffers __wow64_rd64 and its
 * two callers used to allocate fresh on every call.  Each is written and
 * read back before the function that owns it returns, and never held onto
 * after -- nothing here is reentrant or recursive, this whole fix runs
 * straight through on one thread -- so one of each, allocated the first
 * time and reused after, is exactly as correct as a fresh allocation every
 * time.  __wow64_rd64_dword and __wow64_rd64_word are what the module walk
 * and the export walk in this file call for nearly every dword or word read
 * out of a 64-bit image -- thousands of times in one
 * __clone_process_wow64fix () call -- and every one of those was a fresh
 * malloc, never freed.  stage0-pe32 5d32496 measured the identical pattern
 * (there, calloc's own smallest bucket rounding every one of those up to
 * 256 bytes) walking its fixed-size heap section off the end in under 40
 * fork () calls; this port's brk () grows instead of stopping, so the
 * failure mode here is slower rather than absent, not a reason to leave the
 * waste in. */
int __wow64_rd64_have;
int *__wow64_rd64_got;
int *__wow64_rd64_v;
void
__wow64_rd64_init ()
{
  if (__wow64_rd64_have != 0)
    return;
  __wow64_rd64_got = __walloc16 (8);
  __wow64_rd64_v = __walloc16 (4);
  __wow64_rd64_have = 1;
}

/* Read len bytes (at most 8, everything below asks for) from the 64-bit
 * address (lo, hi) of this same process's own 64-bit view, into buf, which
 * the caller owns.  Returns the NTSTATUS from NtWow64ReadVirtualMemory64; a
 * failure leaves buf whatever it was. */
int
__wow64_rd64 (int lo, int hi, int *buf, int len)
{
  int (*NtWow64ReadVirtualMemory64) (int, int, int, int, int, int, int);
  int *got;
  int self;

  self = __wow64_selfhandle ();
  __wow64_rd64_init ();
  got = __wow64_rd64_got;
  got[0] = 0;
  got[1] = 0;
  NtWow64ReadVirtualMemory64 = __ntdll_resolve ("NtWow64ReadVirtualMemory64");
  /* forwards: NtWow64ReadVirtualMemory64 (self, lo, hi, buf, len, 0, got) --
   * self, not -1: see __wow64_selfhandle above. */
  return NtWow64ReadVirtualMemory64 (got, 0, len, buf, hi, lo, self);
}

int
__wow64_rd64_dword (int lo, int hi)
{
  int *v;

  __wow64_rd64_init ();
  v = __wow64_rd64_v;
  v[0] = 0;
  __wow64_rd64 (lo, hi, v, 4);
  return v[0];
}

int
__wow64_rd64_word (int lo, int hi)
{
  int *v;

  __wow64_rd64_init ();
  v = __wow64_rd64_v;
  v[0] = 0;
  __wow64_rd64 (lo, hi, v, 2);
  return 65535 & v[0];
}

/* This process's own 64-bit PEB address (PROCESS_BASIC_INFORMATION64.
 * PebBaseAddress, at +8 into the 48-byte structure, class 0 --
 * ProcessBasicInformation, the same class number the 32-bit
 * NtQueryInformationProcess elsewhere in this port answers for the 32-bit
 * view).  Returns the low word; writes the high word to hi_out, 0/0 on
 * failure. */
int
__wow64_selfpeb (int *hi_out)
{
  int (*NtWow64QueryInformationProcess64) (int, int, int, int, int);
  int *info;
  int *retlen;
  int self;
  int i;

  info = __walloc16 (48);
  i = 0;
  while (i < 12)
    {
      info[i] = 0;
      i = i + 1;
    }
  retlen = __walloc16 (4);
  retlen[0] = 0;
  self = __wow64_selfhandle ();

  NtWow64QueryInformationProcess64 = __ntdll_resolve ("NtWow64QueryInformationProcess64");
  /* forwards: NtWow64QueryInformationProcess64 (self, 0, info, 48, retlen) */
  if (NtWow64QueryInformationProcess64 (retlen, 48, info, 0, self) != 0)
    {
      hi_out[0] = 0;
      return 0;
    }
  hi_out[0] = info[3];
  return info[2];
}

/* Walk this process's own 64-bit PEB -> Ldr -> InLoadOrderModuleList looking
 * for a module named (in ASCII) `want', matched against the wide
 * BaseDllName one UTF-16 code unit at a time.  Returns the module's base
 * (lo), writes the high word to base_hi_out, or returns 0 (writing 0) if the
 * list runs out first.  LDR_DATA_TABLE_ENTRY64 offsets (InLoadOrderLinks
 * +0x00, DllBase +0x30, BaseDllName.Length +0x58, BaseDllName.Buffer +0x60)
 * and PEB64.Ldr (+0x18) are the standard ones. */
int
__wow64_find_module (int peb_lo, int peb_hi, char *want, int *base_hi_out)
{
  int pebldr_lo;
  int pebldr_hi;
  int ldr_lo;
  int ldr_hi;
  int head_lo;
  int head_hi;
  int cur_lo;
  int cur_hi;
  int next_lo;
  int next_hi;
  int base_lo;
  int base_hi;
  int name_len;
  int name_lo;
  int name_hi;
  int guard;
  int i;
  int wc;
  int match;
  int *tmp;

  tmp = malloc (4);

  /* PEB64.Ldr is a pointer at +0x18; +0x18 never carries out of the low
   * word (a PEB address is never within 24 bytes of the top of the address
   * space), so the hi word of that intermediate address is the PEB's own
   * hi word, unchanged. */
  pebldr_lo = __qadd64 (peb_lo, peb_hi, 0x18, tmp);
  pebldr_hi = peb_hi;
  ldr_lo = __wow64_rd64_dword (pebldr_lo, pebldr_hi);
  ldr_hi = __wow64_rd64_dword (__qadd64 (pebldr_lo, pebldr_hi, 4, tmp),
                                pebldr_hi);
  head_lo = __qadd64 (ldr_lo, ldr_hi, 16, tmp);
  head_hi = ldr_hi;             /* +0x10 never carries out of the low word either */
  cur_lo = __wow64_rd64_dword (head_lo, head_hi);
  cur_hi = __wow64_rd64_dword (__qadd64 (head_lo, head_hi, 4, tmp), head_hi);

  guard = 0;
  while (guard < 512)
    {
      if (cur_lo == head_lo && cur_hi == head_hi)
        break;
      if (cur_lo == 0 && cur_hi == 0)
        break;

      base_lo = __wow64_rd64_dword (__qadd64 (cur_lo, cur_hi, 0x30, tmp),
                                     cur_hi);
      base_hi = __wow64_rd64_dword (__qadd64 (cur_lo, cur_hi, 0x34, tmp),
                                     cur_hi);
      name_len = __wow64_rd64_word (__qadd64 (cur_lo, cur_hi, 0x58, tmp),
                                     cur_hi);
      name_lo = __wow64_rd64_dword (__qadd64 (cur_lo, cur_hi, 0x60, tmp),
                                     cur_hi);
      name_hi = __wow64_rd64_dword (__qadd64 (cur_lo, cur_hi, 0x64, tmp),
                                     cur_hi);

      match = 1;
      i = 0;
      while (want[i] != 0)
        {
          if (2 * i >= name_len)
            {
              match = 0;
              break;
            }
          wc = __wow64_rd64_word (__qadd64 (name_lo, name_hi, 2 * i, tmp),
                                   name_hi);
          if (wc != want[i])
            {
              match = 0;
              break;
            }
          i = i + 1;
        }
      if (match != 0 && name_len == 2 * i)
        {
          base_hi_out[0] = base_hi;
          return base_lo;
        }

      /* InLoadOrderLinks.Flink, the first 8 bytes of the current node, both
       * halves read at the CURRENT node's address before either half of
       * cur_lo/cur_hi is overwritten -- reusing the just-updated low word to
       * address the high word would read the wrong place. */
      next_lo = __wow64_rd64_dword (cur_lo, cur_hi);
      next_hi = __wow64_rd64_dword (__qadd64 (cur_lo, cur_hi, 4, tmp),
                                     cur_hi);
      cur_lo = next_lo;
      cur_hi = next_hi;
      guard = guard + 1;
    }
  base_hi_out[0] = 0;
  return 0;
}

/* Resolve one export by name out of a PE32+ image at (base_lo, base_hi) in
 * this process's own 64-bit view.  IMAGE_NT_HEADERS64's export data
 * directory is at +0x88 (the PE32+ optional header, wider than the PE32 one
 * this port's own image uses), and the export directory's four arrays
 * (NumberOfNames +24, AddressOfFunctions +28, AddressOfNames +32,
 * AddressOfNameOrdinals +36) are the ordinary 32-bit-RVA ones documented for
 * every PE image regardless of bitness. */
int
__wow64_resolve_export (int base_lo, int base_hi, char *name, int *va_hi_out)
{
  int e_lfanew;
  int nt_lo;
  int nt_hi;
  int export_rva;
  int num_names;
  int addr_funcs;
  int addr_names;
  int addr_ords;
  int i;
  int name_rva;
  int match;
  int j;
  int ch;
  int ord;
  int func_rva;
  int *tmp;

  tmp = malloc (4);

  e_lfanew = __wow64_rd64_dword (__qadd64 (base_lo, base_hi, 0x3C, tmp),
                                  base_hi);
  nt_lo = __qadd64 (base_lo, base_hi, e_lfanew, tmp);
  nt_hi = base_hi;
  export_rva = __wow64_rd64_dword (__qadd64 (nt_lo, nt_hi, 0x88, tmp), nt_hi);
  num_names = __wow64_rd64_dword (__qadd64 (base_lo, base_hi,
                                             export_rva + 24, tmp), base_hi);
  addr_funcs = __wow64_rd64_dword (__qadd64 (base_lo, base_hi,
                                              export_rva + 28, tmp), base_hi);
  addr_names = __wow64_rd64_dword (__qadd64 (base_lo, base_hi,
                                              export_rva + 32, tmp), base_hi);
  addr_ords = __wow64_rd64_dword (__qadd64 (base_lo, base_hi,
                                             export_rva + 36, tmp), base_hi);

  i = 0;
  while (i < num_names)
    {
      name_rva = __wow64_rd64_dword (__qadd64 (base_lo, base_hi,
                                                 addr_names + 4 * i, tmp),
                                      base_hi);
      match = 1;
      j = 0;
      while (name[j] != 0)
        {
          ch = __wow64_rd64_word (__qadd64 (base_lo, base_hi,
                                             name_rva + j, tmp), base_hi);
          if ((255 & ch) != name[j])
            {
              match = 0;
              break;
            }
          j = j + 1;
        }
      if (match != 0)
        {
          ch = __wow64_rd64_word (__qadd64 (base_lo, base_hi,
                                             name_rva + j, tmp), base_hi);
          if ((255 & ch) == 0)
            {
              ord = __wow64_rd64_word (__qadd64 (base_lo, base_hi,
                                                  addr_ords + 2 * i, tmp),
                                        base_hi);
              func_rva = __wow64_rd64_dword (__qadd64 (base_lo, base_hi,
                                                         addr_funcs + 4 * ord,
                                                         tmp), base_hi);
              va_hi_out[0] = base_hi;
              return __qadd64 (base_lo, base_hi, func_rva, va_hi_out);
            }
        }
      i = i + 1;
    }
  va_hi_out[0] = 0;
  return 0;
}
