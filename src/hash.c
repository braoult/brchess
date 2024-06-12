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
#include <assert.h>

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
 * Initialize all zobrist random bitmasks. Must be called before any other
 * zobrist function, and can be called once only (further calls will be ignored).
 */
void zobrist_init(void)
{
    static bool called = false;
    if (!called) {
        called = true;
        for (color_t c = WHITE; c <= BLACK; ++c) {
            for (piece_type_t p = PAWN; p <= KING; ++p)
                for (square_t sq = A1; sq <= H8; ++sq)
                    zobrist_pieces[MAKE_PIECE(p, c)][sq] = rand64();
        }
        for (castle_rights_t c = CASTLE_NONE; c <= CASTLE_ALL; ++c)
            zobrist_castling[c] = rand64();
        for (file_t f = FILE_A; f <= FILE_H; ++f)
            zobrist_ep[f] = rand64();
        zobrist_ep[8] = 0;                        /* see EP_ZOBRIST_IDX macro */
        zobrist_turn = rand64();
    }
}

/**
 * zobrist_calc() - calculate a position zobrist hash.
 * @pos: &position
 *
 * Normally, Zobrist keys are incrementally calculated when doing or
 * undoing a move.
 * This function should normally only be called:
 * - When starting a new position
 * - To verify incremental Zobrist calculation is correct
 *
 * @return: @pos Zobrist key
 */
hkey_t zobrist_calc(pos_t *pos)
{
    hkey_t key = 0;

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
    key ^= zobrist_castling[pos->castle];
    key ^= zobrist_ep[EP_ZOBRIST_IDX(pos->en_passant)];

    return key;
}

/**
 * zobrist_verify() - verify current position Zobrist key.
 * @pos: &position
 *
 * Verify that position Zobrist key matches a full Zobrist calculation.
 * This function cannot be called if ZOBRIST_VERIFY is not set.
 *
 * @return: True if Zobrist key is OK.
 */
#ifdef ZOBRIST_VERIFY

#pragma push_macro("BUG_ON")                      /* force BUG_ON and WARN_ON */
#pragma push_macro("WARN_ON")
#undef BUG_ON
#define BUG_ON
#undef WARN_ON
#define WARN_ON

bool zobrist_verify(pos_t *pos)
{
    hkey_t diff, key = zobrist_calc(pos);

    if (pos->key == key)
        return true;

    printf("key verify: cur=%#lx != %#lx\n", pos->key, key);

    /* try to find-out the key in different zobrist tables */
    diff = pos->key ^ key;

    for (color_t c = WHITE; c <= BLACK; ++c) {
        for (piece_type_t p = PAWN; p <= KING; ++p)
            for (square_t sq = A1; sq <= H8; ++sq)
                if (diff == zobrist_pieces[MAKE_PIECE(p, c)][sq]) {
                    warn(true, "zobrist difference is piece:[%s][%s]\n",
                         piece_to_fen(MAKE_PIECE(p, c)), sq_to_string(sq));
                    goto end;
                }
    }
    for (castle_rights_t c = CASTLE_NONE; c <= CASTLE_ALL; ++c) {
        if (diff == zobrist_castling[c]) {
            warn(true, "zobrist difference is castling:[%d]\n", c);
            goto end;
        }
    }

    for (file_t f = FILE_A; f <= FILE_H; ++f) {
        if (diff == zobrist_ep[f]) {
            warn(true, "zobrist difference is ep:[%d]\n", f);
            goto end;
        }
    }
    if (diff == zobrist_turn) {
        warn(true, "zobrist difference is turn\n");
        goto end;
    }
    warn(true, "zobrist diff %lx is unknown\n", diff);
end:
    bug_on(false);
    /* not reached */
    return true;
}
#pragma pop_macro("WARN_ON")
#pragma pop_macro("BUG_ON")

#endif

/**
 * tt_create() - create transposition table
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
int tt_create(s32 sizemb)
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
            tt_delete();

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
    tt_clear();

    return hash_tt.nbits;
}

/**
 * tt_clear() - clear transposition table
 *
 * Reset hashtable entries (if available) and statistic information.
 */
void tt_clear()
{
    if (hash_tt.keys)
        memset(hash_tt.keys, 0, hash_tt.bytes);

    hash_tt.used_buckets = 0;
    hash_tt.used_keys = 0;
    hash_tt.collisions = 0;
}

/**
 * tt_delete() - delete transposition table
 *
 * free hashtable data.
 */
void tt_delete()
{
    if (hash_tt.keys)
        safe_free(hash_tt.keys);
    memset(&hash_tt, 0, sizeof(hash_tt));
}

/**
 * tt_probe() - probe tt for an entry
 *
 *
 */
hentry_t *tt_probe(hkey_t key)
{
    bucket_t *bucket;
    hentry_t *entry;
    int i;

    bug_on(!hash_tt.keys);
    bucket = hash_tt.keys + (key & hash_tt.mask);

    /* find key in buckets */
    for (i = 0; i < NBUCKETS; ++i) {
        entry = bucket->entry + i;
        if (key == entry->key)
            break;
    }
    if (i < NBUCKETS)
        return entry;
    return NULL;
}

/**
 * tt_probe_perft() - probe tt for an entry (perft version)
 * @key:   Zobrist (hkey_t) key
 * @depth: depth from search root
 *
 * Search transposition for @key entry with @depth depth.
 *
 * @return: @hentry_t address is found, TT_MISS otherwise.
 */
hentry_t *tt_probe_perft(const hkey_t key, const u16 depth)
{
    bucket_t *bucket;
    hentry_t *entry;
    int i;

    bug_on(!hash_tt.keys);
    bucket = hash_tt.keys + (key & hash_tt.mask);

    /* find key in buckets */
    for (i = 0; i < NBUCKETS; ++i) {
        entry = bucket->entry + i;
        if (key == entry->key && HASH_PERFT_DEPTH(entry->data) == depth) {
            printf("tt hit: key=%lx bucket=%lu entry=%d!\n",
                   key, bucket - hash_tt.keys, i);
            break;
        }
    }
    if (i < NBUCKETS)
        return entry;
    printf("tt miss: key=%lx bucket=%lu\n",
           key, bucket - hash_tt.keys);
    return TT_MISS;
}

/**
 * tt_store_perft() - store a transposition table entry (perft version)
 * @key:   Zobrist (hkey_t) key
 * @depth: depth from search root
 * @nodes: value to store
 *
 */
hentry_t *tt_store_perft(const hkey_t key, const u16 depth, const u64 nodes)
{
    bucket_t *bucket;
    hentry_t *entry;
    int replace = -1, i;
    // uint mindepth = 0;
    u64 data = HASH_PERFT(depth, nodes);
    printf("tt_store: key=%lx data=%lx depth=%d=%d nodes=%lu=%lu\n",
           key, data, depth, HASH_PERFT_DEPTH(data), nodes, HASH_PERFT_VAL(data));
    bug_on(!hash_tt.keys);
    bucket = hash_tt.keys + (key & hash_tt.mask);

    /* find key in buckets */
    for (i = 0; i < NBUCKETS; ++i) {
        entry = bucket->entry + i;
        if (!entry->key) {
            replace = i;
            break;
        }
        /*
         * else {
         *     /\* we replace hash if we are higher in tree *\/
         *     if (key == entry->key && HASH_PERFT_DEPTH(entry->data) > mindepth) {
         *         mindepth = HASH_PERFT_DEPTH(entry->data);
         *         replace = i;
         *     }
         * }
         */
    }
    if (replace >= 0) {
        printf("replacing key=%lx=%lx bucket=%lu idx=%d val=%lu\n",
               key, entry->key, bucket - hash_tt.keys, replace, nodes);
        entry = bucket->entry + replace;
        entry->key = key;
        entry->data = data;
        return entry;
    }
    return NULL;
}
