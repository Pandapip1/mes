/* -*-comment-start: "//";comment-end:""-*-
 * GNU Mes --- Maxwell Equations of Software
 * Copyright © 2016,2017,2018,2019 Jan (janneke) Nieuwenhuizen <janneke@gnu.org>
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

#include <mes/lib.h>
#include <ctype.h>

/* Decimal (and hexadecimal) string to double: the conversion behind atof,
 * strtod, strtof, strtold and scanf's %f.
 *
 * This is not a nicety.  TinyCC's tccpp.c hands every floating-point
 * constant it parses straight to the host libc's strtod, so whichever strtod
 * TinyCC was linked against decides what `1.0e-3' means in every program
 * that compiler ever builds.  When Mes is that host libc -- which is the
 * whole of the bootstrap, up to and including the mainline TinyCC that comes
 * out of it -- this function is where a C constant becomes a number, and an
 * answer that is merely close is an answer that is wrong: DBL_MAX spelled
 * out to its 21 significant digits has to come back as DBL_MAX and not as
 * infinity, or <float.h> stops describing the machine it is on.
 *
 * The version this replaces read the integer part, the fraction and the
 * exponent with abtol -- which accumulates into an int -- and then applied
 * the exponent by multiplying or dividing by ten that many times.  Each of
 * those three steps is lossy in its own way.  A mantissa longer than nine
 * digits wraps the int, so 1.7976931348623157e308 arrived with a negative
 * fraction and came back as -inf.  A fraction was divided by the base
 * exactly once whatever its length, so .25 was a quarter of ten rather than
 * a quarter.  And three hundred separate multiplications by ten round three
 * hundred times, so even 1e308, which the integer part survives, landed one
 * ulp below the true value.
 *
 * So the conversion is done exactly instead.  A decimal input names the
 * value N / D for integers N and D (D is a power of ten when there are
 * digits after the point), and a hexadecimal one names N * 2^e2; the
 * correctly rounded double is obtained by long-dividing N by D as big
 * integers, after scaling so the quotient has exactly the 53 bits a double's
 * significand holds.  The remainder then decides the rounding, with ties
 * broken to even.  That is a single rounding of the exact value, which is
 * what C99 7.20.1.3p9 asks of strtod.
 *
 * Only the first MAXDIG significant digits are kept; anything after them is
 * folded into a sticky flag.  That is exact rather than approximate: a
 * rounding boundary -- a midpoint between two adjacent doubles, so an odd
 * multiple of 2^-1075 at worst -- needs at most 752 significant decimal
 * digits to write down, so it can never fall strictly inside the interval
 * the discarded tail spans.  Either the boundary is exactly the kept prefix,
 * and then the sticky flag says the true value lies above it so we round up,
 * or the kept prefix already decides the rounding for the whole number.
 *
 * Everything below is 32-bit integer arithmetic.  The big integers are held
 * in sixteen-bit limbs so that a limb-by-limb product and its carry still
 * fit in an unsigned, and the 53-bit quotient is carried as two halves:
 * there is no 64-bit int this file can rely on across every compiler that
 * builds this library, MesCC in particular giving `long long' four bytes.
 * The result is assembled from its sign, exponent and significand fields
 * directly rather than scaled into place with ldexp: building it out of
 * floating-point constants would need the very conversion this file exists
 * to provide.
 */

/* Significant digits kept exactly; see above for why this bound makes the
 * conversion exact rather than merely close. */
#define ABTOD_MAXDIG 800

/* Big non-negative integers, sixteen bits to the limb, least significant
 * limb first.  n counts the limbs in use and never leaves a leading zero
 * limb, so n == 0 means the value is zero and the rest of d is stale --
 * which every operation below is careful not to read.
 *
 * The widest intermediate is the divisor of a very small number: ten to the
 * 1123rd, for 800 digits at the bottom of the subnormal range, which is 234
 * limbs, plus the 53 that the quotient's width shifts it by.  360 leaves
 * room over that, at a kilobyte and a half of bss per number. */
#define ABTOD_LIMBS 360

struct abtod_bn
{
  int n;
  unsigned d[ABTOD_LIMBS];
};

/* At file scope, not on the stack.  A PE-targeting TinyCC turns any frame
   over 4096 bytes into a __chkstk call, and the compiler rounds that build
   this library are below the one that can link such a probe -- and a
   bootstrap C library has no business asking a hand-built runtime for eight
   kilobytes of stack in the first place.  Nothing here is reentrant or
   threaded, and abtod_scale is a leaf as far as these five are concerned. */
static struct abtod_bn abtod_N;
static struct abtod_bn abtod_D;
static struct abtod_bn abtod_A;
static struct abtod_bn abtod_B;
static struct abtod_bn abtod_T;

static void
abtod_copy (struct abtod_bn *dst, struct abtod_bn *src)
{
  int i;

  dst->n = src->n;
  i = 0;
  while (i < src->n)
    {
      dst->d[i] = src->d[i];
      i = i + 1;
    }
}

static void
abtod_set (struct abtod_bn *a, unsigned v)
{
  a->d[0] = v;
  if (v != 0)
    a->n = 1;
  else
    a->n = 0;
}

/* a = a * m + c, with m and c both below 2^16 so that a limb's product plus
   the carry stays inside an unsigned: 65535 * 65535 + 65535 is 2^32 - 65536. */
static void
abtod_muladd (struct abtod_bn *a, unsigned m, unsigned c)
{
  unsigned carry;
  unsigned t;
  int i;

  carry = c;
  i = 0;
  while (i < a->n)
    {
      t = a->d[i] * m + carry;
      a->d[i] = t & 0xffff;
      carry = t >> 16;
      i = i + 1;
    }
  while (carry != 0 && a->n < ABTOD_LIMBS)
    {
      a->d[a->n] = carry & 0xffff;
      a->n = a->n + 1;
      carry = carry >> 16;
    }
}

static int
abtod_bits (struct abtod_bn *a)
{
  unsigned v;
  int b;

  if (a->n == 0)
    return 0;
  v = a->d[a->n - 1];
  b = (a->n - 1) * 16;
  while (v != 0)
    {
      b = b + 1;
      v = v >> 1;
    }
  return b;
}

/* The bits a limb shifts off its top are the carry into the next one: this
   is the ordinary shift-with-carry idiom, not an accidental overflow. */
static void
abtod_shl (struct abtod_bn *a, int k)
{
  unsigned carry;
  unsigned t;
  int w;
  int b;
  int i;

  if (a->n == 0 || k <= 0)
    return;
  w = k / 16;
  b = k - w * 16;
  if (b != 0)
    {
      carry = 0;
      i = 0;
      while (i < a->n)
        {
          t = (a->d[i] << b) | carry;
          a->d[i] = t & 0xffff;
          carry = t >> 16;
          i = i + 1;
        }
      if (carry != 0 && a->n < ABTOD_LIMBS)
        {
          a->d[a->n] = carry;
          a->n = a->n + 1;
        }
    }
  if (w != 0)
    {
      /* Cannot happen with the sizes above; do not corrupt memory if it
         somehow does. */
      if (a->n + w > ABTOD_LIMBS)
        w = ABTOD_LIMBS - a->n;
      i = a->n - 1;
      while (i >= 0)
        {
          a->d[i + w] = a->d[i];
          i = i - 1;
        }
      i = 0;
      while (i < w)
        {
          a->d[i] = 0;
          i = i + 1;
        }
      a->n = a->n + w;
    }
}

static void
abtod_shr1 (struct abtod_bn *a)
{
  int i;

  if (a->n == 0)
    return;
  i = 0;
  while (i < a->n - 1)
    {
      a->d[i] = (a->d[i] >> 1) | ((a->d[i + 1] & 1) << 15);
      i = i + 1;
    }
  a->d[a->n - 1] = a->d[a->n - 1] >> 1;
  if (a->d[a->n - 1] == 0)
    a->n = a->n - 1;
}

static int
abtod_cmp (struct abtod_bn *a, struct abtod_bn *b)
{
  int i;

  if (a->n != b->n)
    {
      if (a->n < b->n)
        return -1;
      return 1;
    }
  i = a->n - 1;
  while (i >= 0)
    {
      if (a->d[i] != b->d[i])
        {
          if (a->d[i] < b->d[i])
            return -1;
          return 1;
        }
      i = i - 1;
    }
  return 0;
}

/* a -= b; the caller guarantees a >= b.  A limb's difference is held in a
   signed int, where it lands between -65535 and 65535, and its sign is the
   borrow into the next limb. */
static void
abtod_sub (struct abtod_bn *a, struct abtod_bn *b)
{
  int borrow;
  int t;
  int i;

  borrow = 0;
  i = 0;
  while (i < a->n)
    {
      t = a->d[i] - borrow;
      if (i < b->n)
        t = t - b->d[i];
      if (t < 0)
        {
          t = t + 0x10000;
          borrow = 1;
        }
      else
        borrow = 0;
      a->d[i] = t;
      i = i + 1;
    }
  while (a->n != 0 && a->d[a->n - 1] == 0)
    a->n = a->n - 1;
}

/* a *= 10^e (e >= 0), as 5^e -- in chunks of 5^6, which is 15625 and so
   still a legal multiplier for abtod_muladd -- followed by an exact shift. */
static void
abtod_mul_pow10 (struct abtod_bn *a, int e)
{
  unsigned m;
  int k;

  k = e;
  while (k >= 6)
    {
      abtod_muladd (a, 15625, 0);
      k = k - 6;
    }
  if (k != 0)
    {
      m = 1;
      while (k != 0)
        {
          m = m * 5;
          k = k - 1;
        }
      abtod_muladd (a, m, 0);
    }
  abtod_shl (a, e);
}

/* An IEEE 754 binary64 built from its fields.  The significand arrives as
   the two halves of the 53-bit integer q -- hi holding bits 32 to 52 -- and
   the value is q * 2^g.  Writing the two words and reading the double back
   out is how a bootstrap C library names a number it cannot spell as a
   constant; every machine Mes targets is little-endian, so the low word
   comes first. */
static double
abtod_double (unsigned hi, unsigned lo, int g, int neg)
{
  union
  {
    double d;
    unsigned w[2];
  } u;
  unsigned top;
  int biased;

  if (hi == 0 && lo == 0)
    biased = 0;
  else if ((hi & 0x100000) != 0)
    {
      /* q >= 2^52, so the number is normal and that bit is the implicit
         one, which the mask below drops. */
      biased = g + 1075;
      if (biased >= 2047)
        {
          hi = 0;
          lo = 0;
          biased = 2047;
        }
    }
  else
    /* Fewer than 53 bits, which abtod_scale only produces once g has
       bottomed out at -1074: a subnormal, whose exponent field is zero and
       whose significand is the whole of q. */
    biased = 0;

  top = (hi & 0xfffff) | ((unsigned) biased << 20);
  if (neg != 0)
    top = top | 0x80000000;
  u.w[0] = lo;
  u.w[1] = top;
  return u.d;
}

static double
abtod_inf (int neg)
{
  return abtod_double (0x100000, 0, 1024, neg);
}

static double
abtod_zero (int neg)
{
  return abtod_double (0, 0, 0, neg);
}

/* The correctly rounded value of (N / D) * 2^e2, as a double.  sticky says
   the true value exceeds N / D * 2^e2 by less than one unit in the last
   place N keeps.  N and D must both be nonzero. */
static double
abtod_scale (struct abtod_bn *N, struct abtod_bn *D, int e2, int sticky,
             int neg)
{
  unsigned qhi;
  unsigned qlo;
  int k;
  int g;
  int t;
  int i;
  int c;

  /* The value lies between 2^(k-1) and 2^(k+1). */
  k = abtod_bits (N) - abtod_bits (D) + e2;
  if (k >= 1025)
    return abtod_inf (neg);
  if (k <= -1076)
    return abtod_zero (neg);

  /* Find the scale g at which floor(N * 2^(e2-g) / D) has exactly 53 bits --
     or fewer, but only once g has bottomed out at the smallest exponent a
     double has, which is what a subnormal result means.  k is already within
     one of the answer, so this settles in a step or two. */
  g = k - 53;
  if (g < -1074)
    g = -1074;
  while (1)
    {
      abtod_copy (&abtod_A, N);
      abtod_copy (&abtod_B, D);
      t = e2 - g;
      if (t > 0)
        abtod_shl (&abtod_A, t);
      else
        abtod_shl (&abtod_B, -t);
      abtod_copy (&abtod_T, &abtod_B);
      abtod_shl (&abtod_T, 53);
      if (abtod_cmp (&abtod_A, &abtod_T) >= 0)
        {
          g = g + 1;
          continue;
        }
      if (g > -1074)
        {
          abtod_copy (&abtod_T, &abtod_B);
          abtod_shl (&abtod_T, 52);
          if (abtod_cmp (&abtod_A, &abtod_T) < 0)
            {
              g = g - 1;
              continue;
            }
        }
      break;
    }

  /* Restoring long division, most significant bit first: q becomes
     floor(A / B) and A becomes the remainder. */
  abtod_copy (&abtod_T, &abtod_B);
  abtod_shl (&abtod_T, 52);
  qhi = 0;
  qlo = 0;
  i = 52;
  while (i >= 0)
    {
      qhi = (qhi << 1) | (qlo >> 31);
      qlo = qlo << 1;
      if (abtod_cmp (&abtod_A, &abtod_T) >= 0)
        {
          abtod_sub (&abtod_A, &abtod_T);
          qlo = qlo | 1;
        }
      abtod_shr1 (&abtod_T);
      i = i - 1;
    }

  /* Round to nearest, ties to even; a sticky tail beats the tie. */
  abtod_shl (&abtod_A, 1);
  c = abtod_cmp (&abtod_A, &abtod_B);
  if (c > 0 || (c == 0 && (sticky != 0 || (qlo & 1) != 0)))
    {
      qlo = qlo + 1;
      if (qlo == 0)
        qhi = qhi + 1;
      if ((qhi & 0x200000) != 0)
        {
          /* The carry ran off the top: 2^53 is 2^52 one binade up. */
          qlo = (qlo >> 1) | (qhi << 31);
          qhi = qhi >> 1;
          g = g + 1;
        }
    }
  return abtod_double (qhi, qlo, g, neg);
}

static int
abtod_hexdigit (int c)
{
  if (c >= '0' && c <= '9')
    return c - '0';
  if (c >= 'a' && c <= 'f')
    return c - 'a' + 10;
  if (c >= 'A' && c <= 'F')
    return c - 'A' + 10;
  return -1;
}

double
abtod (char const **p, int base)
{
  unsigned char dig[ABTOD_MAXDIG];
  char const *s;
  char const *start;
  char const *q;
  unsigned chunk;
  unsigned mul;
  int neg;
  int nd;
  int exp;
  int any;
  int seen_dot;
  int sticky;
  int e;
  int eneg;
  int d;
  int i;
  int j;

  s = *p;
  start = s;
  if (base == 0)
    base = 10;

  while (isspace (s[0]) != 0)
    s = s + 1;
  neg = 0;
  if (s[0] == '+')
    s = s + 1;
  else if (s[0] == '-')
    {
      neg = 1;
      s = s + 1;
    }

  /* Digits, most significant first, with the point folded into exp: a
     decimal digit is worth one to it and a hexadecimal digit four, since a
     hexadecimal float's exponent counts in bits.  Leading zeros are not
     significant digits, and once MAXDIG of them are in hand the rest only
     move the point and set the sticky flag. */
  nd = 0;
  exp = 0;
  any = 0;
  seen_dot = 0;
  sticky = 0;
  while (1)
    {
      if (s[0] == '.' && seen_dot == 0)
        {
          seen_dot = 1;
          s = s + 1;
          continue;
        }
      if (base == 16)
        d = abtod_hexdigit (s[0]);
      else if (s[0] >= '0' && s[0] <= '9')
        d = s[0] - '0';
      else
        d = -1;
      if (d < 0)
        break;
      any = 1;
      if (nd < ABTOD_MAXDIG)
        {
          if (nd != 0 || d != 0)
            {
              dig[nd] = d;
              nd = nd + 1;
            }
          if (seen_dot != 0)
            {
              if (base == 16)
                exp = exp - 4;
              else
                exp = exp - 1;
            }
        }
      else
        {
          if (d != 0)
            sticky = 1;
          if (seen_dot == 0)
            {
              if (base == 16)
                exp = exp + 4;
              else
                exp = exp + 1;
            }
        }
      s = s + 1;
    }
  if (any == 0)
    {
      /* Nothing here that could be a number; leave *p where it was. */
      *p = start;
      return abtod_zero (0);
    }

  /* The exponent, if one is spelled out and spelled out completely: an `e'
     or `p' with no digits behind it is not part of the number, and the
     caller is entitled to see it again. */
  e = 0;
  eneg = 0;
  if ((base == 10 && (s[0] == 'e' || s[0] == 'E'))
      || (base == 16 && (s[0] == 'p' || s[0] == 'P')))
    {
      q = s + 1;
      if (q[0] == '+')
        q = q + 1;
      else if (q[0] == '-')
        {
          eneg = 1;
          q = q + 1;
        }
      if (q[0] >= '0' && q[0] <= '9')
        {
          while (q[0] >= '0' && q[0] <= '9')
            {
              /* Anything past this is already past every finite double, and
                 stopping keeps the shifts below from running away. */
              if (e < 100000)
                e = e * 10 + q[0] - '0';
              q = q + 1;
            }
          s = q;
        }
      else
        eneg = 0;
    }
  if (eneg != 0)
    exp = exp - e;
  else
    exp = exp + e;
  *p = s;

  if (nd == 0)
    return abtod_zero (neg);

  /* Trailing zeros of the kept prefix only make the big integers wider. */
  while (nd > 1 && dig[nd - 1] == 0)
    {
      nd = nd - 1;
      if (base == 16)
        exp = exp + 4;
      else
        exp = exp + 1;
    }

  if (base == 16)
    {
      /* Hexadecimal digits are exact in binary, so the mantissa is one big
         integer and the exponent is already a bit count. */
      abtod_set (&abtod_N, 0);
      i = 0;
      while (i < nd)
        {
          abtod_muladd (&abtod_N, 16, dig[i]);
          i = i + 1;
        }
      abtod_set (&abtod_D, 1);
      return abtod_scale (&abtod_N, &abtod_D, exp, sticky, neg);
    }

  /* The value lies between 10^(exp+nd-1) and 10^(exp+nd), which settles the
     two extremes without building anything: 10^309 is already past DBL_MAX,
     and 10^-324 is below half the smallest subnormal.  This also keeps an
     exponent like 1e99999 from asking for a shift of a hundred thousand
     bits. */
  if (exp + nd >= 310)
    return abtod_inf (neg);
  if (exp + nd <= -324)
    return abtod_zero (neg);

  abtod_set (&abtod_N, 0);
  i = 0;
  while (i < nd)
    {
      chunk = 0;
      mul = 1;
      j = 0;
      while (j < 4 && i < nd)
        {
          chunk = chunk * 10 + dig[i];
          mul = mul * 10;
          j = j + 1;
          i = i + 1;
        }
      abtod_muladd (&abtod_N, mul, chunk);
    }
  abtod_set (&abtod_D, 1);
  if (exp > 0)
    abtod_mul_pow10 (&abtod_N, exp);
  else if (exp < 0)
    abtod_mul_pow10 (&abtod_D, -exp);
  return abtod_scale (&abtod_N, &abtod_D, 0, sticky, neg);
}
