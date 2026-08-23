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

#include <math.h>

/* value * 2^exponent, replacing the stub that returned zero.
 *
 * Like abtod, this one is load-bearing for the compiler above it rather than
 * only for programs: TinyCC parses a hexadecimal floating constant by hand
 * and calls ldexpl to put the mantissa at its exponent, and tcc.h maps
 * ldexpl onto ldexp on Windows, where long double is a double.  With a stub
 * underneath, every hexadecimal float constant in every program that
 * compiler builds came out as zero -- silently, since 0x1p64 is a perfectly
 * well-formed thing to write and nothing downstream can tell it was meant to
 * be a power of two.
 *
 * It is written entirely in integer arithmetic, over the two halves of the
 * IEEE 754 binary64 the argument already is.  The obvious implementation --
 * repeated multiplication by two, which is what lib/math/ldexpl.c does --
 * loses on both ends: it rounds once per step through the subnormal range,
 * and it needs a working floating-point multiply from a compiler round that
 * may not have one yet.  Shifting the significand rounds exactly once, in
 * the one place C requires it to.
 */

union ldexp_bits
{
  double d;
  unsigned w[2];
};

double
ldexp (double value, int exponent)
{
  union ldexp_bits u;
  unsigned sign;
  unsigned hi;
  unsigned lo;
  unsigned round;
  unsigned sticky;
  int biased;
  int e;
  int s;
  int i;

  u.d = value;
  /* Every machine Mes targets is little-endian, so w[1] is the half with
     the sign and the exponent in it. */
  sign = u.w[1] & 0x80000000;
  biased = (u.w[1] >> 20) & 0x7ff;
  hi = u.w[1] & 0xfffff;
  lo = u.w[0];

  if (biased == 0x7ff)
    return value;               /* an infinity or a NaN scales to itself */
  if (biased == 0 && hi == 0 && lo == 0)
    return value;               /* a zero, whose sign is kept */

  /* One representation for both kinds of argument: a 53-bit significand in
     hi:lo, worth hi:lo * 2^e.  A normal number's leading one is implicit and
     put back here; a subnormal's is simply absent, and the loop below
     supplies the normalisation the exponent field could not. */
  if (biased == 0)
    e = -1074;
  else
    {
      hi = hi | 0x100000;
      e = biased - 1075;
    }

  /* Clamped so that the addition cannot wrap an int, and so that the shift
     counts below stay small.  Anything this size already saturates. */
  if (exponent > 5000)
    exponent = 5000;
  if (exponent < -5000)
    exponent = -5000;
  e = e + exponent;

  /* Normalise, so that the tests against the format's limits are about the
     value and not about how it happened to be written. */
  while ((hi & 0x100000) == 0 && (hi != 0 || lo != 0))
    {
      hi = (hi << 1) | (lo >> 31);
      lo = lo << 1;
      e = e - 1;
    }
  if (hi == 0 && lo == 0)
    {
      u.w[0] = 0;
      u.w[1] = sign;
      return u.d;
    }

  if (e > 971)
    {
      /* Past the largest exponent a double has: an infinity of the same
         sign, which is what the stub could never say. */
      u.w[0] = 0;
      u.w[1] = sign | 0x7ff00000;
      return u.d;
    }

  if (e >= -1074)
    {
      u.w[0] = lo;
      u.w[1] = sign | ((unsigned) (e + 1075) << 20) | (hi & 0xfffff);
      return u.d;
    }

  /* Subnormal: the significand moves right until its exponent is the
     smallest a double has, and the bits that fall off decide the rounding --
     to nearest, ties to even.  Fifty-four steps shift even the round bit
     away, so nothing but a signed zero can be left. */
  s = -1074 - e;
  if (s > 54)
    {
      u.w[0] = 0;
      u.w[1] = sign;
      return u.d;
    }
  round = 0;
  sticky = 0;
  i = 0;
  while (i < s)
    {
      if (round != 0)
        sticky = 1;
      round = lo & 1;
      lo = (lo >> 1) | (hi << 31);
      hi = hi >> 1;
      i = i + 1;
    }
  if (round != 0 && (sticky != 0 || (lo & 1) != 0))
    {
      lo = lo + 1;
      if (lo == 0)
        hi = hi + 1;
    }
  /* hi is at most 0x100000 here, and that bit lands in the exponent field
     exactly where it belongs: a significand that rounded up to 2^52 is the
     smallest normal number. */
  u.w[0] = lo;
  u.w[1] = sign | hi;
  return u.d;
}
