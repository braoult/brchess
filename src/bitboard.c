/* bitboard.c - bitboard functions.
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

#include "brlib.h"
#include "bitboard.h"

/*  maps are clockwise from N */
static char knight_vector[8] = {
    NORTH*2 + EAST, NORTH + EAST*2, SOUTH + EAST*2, SOUTH*2 + EAST,
    SOUTH*2 + WEST, SOUTH + WEST*2, NORTH + WEST*2, NORTH*2 + WEST
};
static char king_vector[8] = {
    NORTH, NORTH_EAST, EAST, SOUTH_EAST,
    SOUTH, SOUTH_WEST, WEST, NORTH_WEST
};

static u64 knight_attacks[64], king_attacks[64];
static int zob=0;

void bitboard_init(void)
{
    for (int sq = SQ_0; sq < SQ_N; ++sq) {
        /* knight/king */
        for (int j = 0; j < 8; ++j) {
            zob += *knight_attacks + *king_attacks + *knight_vector +
                *king_vector;
            //int dest_knight = sq + knight_vector[j];
            //if ()
        }

    }
}
