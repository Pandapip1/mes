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

#ifndef __MES_WINDOWS_NTDLL_H
#define __MES_WINDOWS_NTDLL_H

/* Which ntdll routine lib/m2/x86/ntdll-i386.hex2 resolved into which slot.
 * The numbering is decided there and nowhere else, so this half is copied
 * from stage0-pe32 a677ddb (x86/M2libc-windows/ntdll-slots.h), which that
 * project generates from the same list it generates the .hex2 from.  If the
 * .hex2 beside it is updated, this must be too.
 */
#define NT_CREATE      0   /* NtCreateFile: open a file, or make one */
#define NT_READ        1   /* NtReadFile: read */
#define NT_WRITE       2   /* NtWriteFile: write */
#define NT_CLOSE       3   /* NtClose: close, and any other handle */
#define NT_EXIT        4   /* NtTerminateProcess: _exit */
#define NT_RTLPATH     5   /* RtlDosPathNameToNtPathName_U: a DOS path becomes an NT path */
#define NT_SETINFO     6   /* NtSetInformationFile: lseek, and unlink */
#define NT_QUERYINFO   7   /* NtQueryInformationFile: lseek, and fstat */
#define NT_QUERYATTR   8   /* NtQueryAttributesFile: access */
#define NT_QUERYFULL   9   /* NtQueryFullAttributesFile: stat */
#define NT_SETCWD      10  /* RtlSetCurrentDirectory_U: chdir */
#define NT_GETCWD      11  /* RtlGetCurrentDirectory_U: getcwd */
#define NT_DUP         12  /* NtDuplicateObject: dup and dup2 */
#define NT_TIME        13  /* NtQuerySystemTime: time, gettimeofday, clock_gettime */
#define NT_QUERYVOL    14  /* NtQueryVolumeInformationFile: isatty */
#define NT_WAIT        15  /* NtWaitForSingleObject: waitpid */
#define NT_QUERYPROC   16  /* NtQueryInformationProcess: waitpid, for the exit status */
#define NT_DELETE      17  /* NtDeleteFile: unlink and rmdir */
#define NT_VERSION     18  /* RtlGetVersion: uname */
#define NT_MAKEPARAMS  19  /* RtlCreateProcessParameters: __spawn, for the child's parameter block */
#define NT_CREATEPROC  20  /* RtlCreateUserProcess: __spawn */
#define NT_RESUME      21  /* NtResumeThread: __spawn: a new process starts suspended */
#define NT_ALLOC       22  /* NtAllocateVirtualMemory: a real brk, for a program too big to live inside the image */

int *__iosb ();
void *__ntdll (int slot);
int *__stdslot (int n);
int __handle (int filedes);
char *__widen (char const *s);
int *__dosustring (char const *path);
int *__ntobject (char const *path);
int __strput (char *dst, int at, char const *src);
int __spawn (char const *file_name, char **argv, char **env);

#endif /* __MES_WINDOWS_NTDLL_H */
