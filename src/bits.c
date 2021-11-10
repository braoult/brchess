/* bits.c - bits.h tests
 *
 * Copyright (C) 2021 Bruno Raoult ("br")
 * Licensed under the GNU General Public License v3.0 or later.
 * Some rights reserved. See COPYING.
 *
 * You should have received a copy of the GNU General Public License along with this
 * program. If not, see <https://www.gnu.org/licenses/gpl-3.0-standalone.htmlL>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later <https://spdx.org/licenses/GPL-3.0-or-later.html>
 *
 */

#include "bits.h"

#ifdef BIN_bits
#include <stdio.h>
#include <stdlib.h>

static inline int _popcount64(u64 n)
{
    int count = 0;
    while (n) {
        count++;
        n &= (n - 1);
    }
    return count;
}

static inline int _ctz64(u64 n)
{
    return _popcount64((n & -n) - 1);
}

static inline int _clz64(u64 n)
{
    u64 r, q;

    r = (n > 0xFFFFFFFF) << 5; n >>= r;
    q = (n > 0xFFFF)     << 4; n >>= q; r |= q;
    q = (n > 0xFF  )     << 3; n >>= q; r |= q;
    q = (n > 0xF   )     << 2; n >>= q; r |= q;
    q = (n > 0x3   )     << 1; n >>= q; r |= q;
    r |= (n >> 1);
   return __WORDSIZE - r - 1;
}

static inline int _ffs64(u64 n)
{
    if (n == 0)
        return (0);

    return _popcount64(n ^ ~-n);
}


int main(int ac, char **av)
{
    u64 u = 123, _tmp;
    int curbit;
    int base = 10;
    debug_init(0);
    if (ac > 2)
        base = atoi(*(av+2));
    if (ac > 1) {
        u = strtoul(*(av+1), NULL, base);
        printf("base=%d input=%#lx\n", base, u);
        printf("popcount64(%lu) = %d/%d\n", u, popcount64(u), _popcount64(u));
        printf("ctz64(%lu) = %d/%d\n", u, ctz64(u), _ctz64(u));
        printf("clz64(%lu) = %d/%d\n", u, clz64(u), _clz64(u));
        printf("ffs64(%lu) = %d/%d\n", u, ffs64(u), _ffs64(u));
        printf("\n");

        bit_for_each64(curbit, _tmp, u) {
            printf("loop: curbit=%d tmp=%ld\n", curbit, _tmp);
        }
        printf("\n");
        bit_for_each64_2(curbit, _tmp, u) {
            printf("loop2: curbit=%d tmp=%ld\n", curbit, _tmp);
        }

    }
    return 0;
}
#endif  /* BIN_bits */
