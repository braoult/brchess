/* hash.c - hash management.
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

#include <string.h>

#include <brlib.h>
#include <bitops.h>

#include "chessdefs.h"
#include "util.h"
#include "position.h"
#include "piece.h"
#include "hash.h"

u64 zobrist_pieces[16][64];
u64 zobrist_castling[4 * 4 + 1];
u64 zobrist_turn;                                 /* for black, XOR each ply */
u64 zobrist_ep[9];                                /* 0-7: ep file, 8: SQUARE_NONE */

hasht_t hash_tt;                                  /* main transposition table */

/**
 * zobrist_init() - initialize zobrist tables.
 *
 * Initialize all zobrist random bitmasks.
 */
void zobrist_init(void)
{
    for (color_t c = WHITE; c <= BLACK; ++c) {
        for (piece_type_t p = PAWN; p <= KING; ++p)
            for (square_t sq = A1; sq <= H8; ++sq)
                zobrist_pieces[MAKE_PIECE(p, c)][sq] = rand64();
    }
    for (castle_rights_t c = CASTLE_NONE; c <= CASTLE_ALL; ++c)
        zobrist_castling[c] = rand64();
    for (file_t f = FILE_A; f <= FILE_H; ++f)
        zobrist_ep[f] = rand64();
    zobrist_ep[8] = 0;
    zobrist_turn = rand64();
}

/**
 * zobrist_calc() - calculate a position zobrist hash.
 *
 * Normally, Zobrist keys are incrementally calculated when doing or
 * undoing a move.
 * This function should normally only be called:
 * - When starting a new position
 * - To verify incremental Zobrist calculation is correct
 *
 */
key_t zobrist_calc(pos_t *pos)
{
    key_t key = 0;

    if (pos->turn == BLACK)
        key ^= zobrist_turn;

    for (color_t c = WHITE; c <= BLACK; ++c) {
        for (piece_type_t pt = PAWN; pt <= KING; ++pt) {
            piece_t piece = MAKE_PIECE(pt, c);
            bitboard_t bb = pos->bb[c][pt];
            while (bb) {
                square_t sq = bb_next(&bb);
                key ^= zobrist_pieces[piece][sq];
            }
        }
    }
    key ^= pos->castle;
    key ^= EP_ZOBRIST_IDX(pos->en_passant);

    return key;
}

/**
 * hash_create() - hashtable creation.
 * @sizemb: s32 size of hash table in Mb
 *
 * Create a hash table of max @sizemb (or HASH_SIZE_MBif @sizemb <= 0) Mb size.
 * This function must be called at startup.
 *
 * The number of bucket_t entries fitting in @sizemb is calculated, and rounded
 * (down) to a power of 2.
 * This means the actual size could be lower than @sizemb (nearly halved in
 * worst case).
 *
 * If transposition hashtable already exists and new size would not change,
 * the old one is cleared.
 * If transposition hashtable already exists and new size is different,
 * the old one is destroyed first (old data is not preserved).
 *
 * TODO:
 * - Rebuild old hashtable data ?
 *
 * @return: hash table size in Mb. If memory allocation fails, the function does
 * not return.
 */
int hash_create(s32 sizemb)
{
    size_t bytes, target_nbuckets;
    u32 nbits;

    static_assert(sizeof(hentry_t) == 16, "fatal: hentry_t size != 16");

    //printf("mb = %'7u ", sizemb);
    /* adjust tt size */
    if (sizemb <= 0)
        sizemb = HASH_SIZE_DEFAULT;
    sizemb = clamp(sizemb, HASH_SIZE_MIN, HASH_SIZE_MAX);
    //printf("-> %'6d ", sizemb);

    bytes = sizemb * 1024ull * 1024ull;           /* bytes wanted */
    target_nbuckets = bytes / sizeof(bucket_t);   /* target buckets */

    nbits = msb64(target_nbuckets);               /* adjust to power of 2 */

    if (hash_tt.nbits != nbits) {
        if (hash_tt.nbits)
            hash_delete();

        hash_tt.nbits    = nbits;

        hash_tt.nbuckets = BIT(hash_tt.nbits);
        hash_tt.nkeys    = hash_tt.nbuckets * NBUCKETS;

        hash_tt.bytes    = hash_tt.nbuckets * sizeof(bucket_t);
        hash_tt.mb       = hash_tt.bytes / 1024 / 1024;

        hash_tt.mask     = -1ull >> (64 - nbits);

        hash_tt.keys     = safe_malloc(hash_tt.bytes);

        //printf("bits=%2d size=%'15lu/%'6d Mb/%'14lu buckets ",
        //       hash_tt.nbits, hash_tt.bytes, hash_tt.mb, hash_tt.nbuckets);
        //printf("mask=%9x\n", hash_tt.mask);
    }
    //else {
    //    printf("unchanged (cleared)\n");
    //}
    /* attention - may fail ! */
    hash_clear();

    return hash_tt.nbits;
}

/**
 * hash_clear() - clear hashtable data.
 *
 * Reset hashtable entries (if available) and statistic information.
 */
void hash_clear()
{
    if (hash_tt.keys)
        memset(hash_tt.keys, 0, hash_tt.bytes);

    hash_tt.used_buckets = 0;
    hash_tt.used_keys = 0;
    hash_tt.collisions = 0;
}

/**
 * hash_delete() - delete hashtable data.
 *
 * free hashtable data.
 */
void hash_delete()
{
    if (hash_tt.keys)
        safe_free(hash_tt.keys);
    memset(&hash_tt, 0, sizeof(hash_tt));
}
