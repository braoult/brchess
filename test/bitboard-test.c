/* bitboard-test.c - basic bitboard/hyperbola tests.
 *
 * Copyright (C) 2024 Bruno Raoult ("br")
 * Licensed under the GNU General Public License v3.0 or later.
 * Some rights reserved. See COPYING.
 *
 * You should have received a copy of the GNU General Public License along with this
 * program. If not, see <https://www.gnu.org/licenses/gpl-3.0-standalone.html>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later <https://spdx.org/licenses/GPL-3.0-or-later.html>
 *
 */

#include <stdio.h>
#include <string.h>

#include "bug.h"

#include "chessdefs.h"
#include "bitboard.h"
#include "hq.h"

int main(int __unused ac, __unused char**av)
{
    char str[256];
    bitboard_init();
    hyperbola_init();
    for (int i = 0; i < 64; ++i) {
        sprintf(str, "\n%#x:\n   %-22s%-22s%-22s%-22s%-22s%-22s%-22s", i,
                "sliding", "diagonal", "antidiagonal", "file", "rank", "knight",
                "king"
                );
        bb_print_multi(str, 7,
                             bb_sqfile[i] | bb_sqrank[i] |
                             bb_sqdiag[i] | bb_sqanti[i],
                             bb_sqdiag[i], bb_sqanti[i],
                             bb_sqfile[i], bb_sqrank[i],
                             bb_knight[i], bb_king[i]);
    }
    sprintf(str, "between: %-22s%-22s%-22s%-22s%-22s%-22s",
            "a1-a8", "a1-h8", "a1-h1", "a2-a7", "a2-g7", "a2-g2");
    bb_print_multi(str, 6,
                         bb_between[A1][A8], bb_between[A1][H8],
                         bb_between[A1][H1], bb_between[A2][A7],
                         bb_between[A2][G7], bb_between[A2][G2]);
    sprintf(str, "between: %-22s%-22s%-22s%-22s%-22s%-22s%-22s%-22s",
            "c3-c6", "c3-f6", "c3-f3", "c3-e1", "c3-c1", "c3-a1", "c3-a3", "c3-a5");
    bb_print_multi(str, 8,
                   bb_between[C3][C6], bb_between[C3][F6],
                   bb_between[C3][F3], bb_between[C3][E1],
                   bb_between[C3][C1], bb_between[C3][A1],
                   bb_between[C3][A3], bb_between[C3][A5]);
    sprintf(str, "between: %-22s%-22s%-22s%-22s%-22s%-22s%-22s%-22s",
            "c4-c6", "c4-f6", "c4-f3", "c4-e1", "c4-c1", "c4-a1", "c4-a3", "c4-a5");
    bb_print_multi(str, 8,
                   bb_between[C4][C6], bb_between[C4][F6],
                   bb_between[C4][F3], bb_between[C4][E1],
                   bb_between[C4][C1], bb_between[C4][A1],
                   bb_between[C4][A3], bb_between[C4][A5]);
    sprintf(str, "Pwn att: %-22s%-22s%-22s%-22s%-22s%-22s%-22s%-22s",
            "White a2", "Black a2", "White h7", "Black h7",
            "White c3", "Black c3", "White e5", "Black e5");
    bb_print_multi(str, 8,
                   bb_pawn_attacks[WHITE][A2], bb_pawn_attacks[BLACK][A2],
                   bb_pawn_attacks[WHITE][H7], bb_pawn_attacks[BLACK][H7],
                   bb_pawn_attacks[WHITE][C3], bb_pawn_attacks[BLACK][C3],
                   bb_pawn_attacks[WHITE][E5], bb_pawn_attacks[BLACK][E5]);
    return 0;
}
