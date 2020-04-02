/* Copyright (c) 2018 SiFive, Inc */
/* SPDX-License-Identifier: Apache-2.0 */
/* SPDX-License-Identifier: GPL-2.0-or-later */
/* See the file LICENSE for further information */

/* Copyright (c) 2017  SiFive Inc. All rights reserved.

   This copyrighted material is made available to anyone wishing to use,
   modify, copy, or redistribute it subject to the terms and conditions
   of the FreeBSD License.   This program is distributed in the hope that
   it will be useful, but WITHOUT ANY WARRANTY expressed or implied,
   including the implied warranties of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  A copy of this license is available at
   http://www.opensource.org/licenses.
*/

#include <string.h>
#include <stdint.h>

#define unlikely(X) __builtin_expect (!!(X), 0)
extern int memcmp1(const void* s1, const void* s2, size_t n);
void *
memcpy1(void *__restrict aa, const void *__restrict bb, size_t n)
{
  #define BODY(a, b, t) { \
    t tt = *b; \
    a++, b++; \
    *(a - 1) = tt; \
  }

  char *a = (char *)aa;
  const char *b = (const char *)bb;
  char *end = a + n;
  uintptr_t msk = sizeof (long) - 1;
  if (unlikely ((((uintptr_t)a & msk) != ((uintptr_t)b & msk))
	       || n < sizeof (long)))
    {
small:
      if (__builtin_expect (a < end, 1))
	while (a < end)
	  BODY (a, b, char);
      return aa;
    }

  if (unlikely (((uintptr_t)a & msk) != 0))
    while ((uintptr_t)a & msk)
      BODY (a, b, char);

  long *la = (long *)a;
  const long *lb = (const long *)b;
  long *lend = (long *)((uintptr_t)end & ~msk);

  if (unlikely (la < (lend - 8)))
    {
      while (la < (lend - 8))
	{
	  long b0 = *lb++;
	  long b1 = *lb++;
	  long b2 = *lb++;
	  long b3 = *lb++;
	  long b4 = *lb++;
	  long b5 = *lb++;
	  long b6 = *lb++;
	  long b7 = *lb++;
	  long b8 = *lb++;
	  *la++ = b0;
	  *la++ = b1;
	  *la++ = b2;
	  *la++ = b3;
	  *la++ = b4;
	  *la++ = b5;
	  *la++ = b6;
	  *la++ = b7;
	  *la++ = b8;
	}
    }

  while (la < lend)
    BODY (la, lb, long);

  a = (char *)la;
  b = (const char *)lb;
  if (unlikely (a < end))
    goto small;
  return aa;
}
int memcmp1(const void* s1, const void* s2, size_t n)
{
  if ((((uintptr_t)s1 | (uintptr_t)s2) & (sizeof(uintptr_t)-1)) == 0) {
    const uintptr_t* u1 = s1;
    const uintptr_t* u2 = s2;
    const uintptr_t* end = u1 + (n / sizeof(uintptr_t));
    while (u1 < end) {
      if (*u1 != *u2)
        break;
      u1++;
      u2++;
    }
    n -= (const void*)u1 - s1;
    s1 = u1;
    s2 = u2;
  }

  while (n--) {
    unsigned char c1 = *(const unsigned char*)s1++;
    unsigned char c2 = *(const unsigned char*)s2++;
    if (c1 != c2)
      return c1 - c2;
  }

  return 0;
}
