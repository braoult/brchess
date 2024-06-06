/* hash.h - hash management.
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

#ifndef HASH_H
#define HASH_H

#include <assert.h>

#include "chessdefs.h"

#define NBUCKETS             4                    /* buckets per hash table entry */

#define HASH_SIZE_DEFAULT   32                    /* default: 32Mb */
#define HASH_SIZE_MIN        4
#define HASH_SIZE_MAX    32768                    /* 32Gb */

typedef u64 hkey_t;                               /* cannot use typedef for key_t */

/**
 * hentry_t: hashtable bucket.
 *
 * Size should be exactly 16 bytes. If impossible to fit necessary data in
 * 16 bytes in future, it should be updated to be exactly 32 bytes.
 */
typedef struct {
    hkey_t key;                                    /* zobrist */
    union {
        u64 data;
        struct {
            u16 depth;                            /* ply in search */
            s16 eval;
            u16 move;
            u8 flags;                             /* maybe for locking, etc... */
            u8 filler;
        };
    };
} hentry_t;

typedef struct {
    hentry_t buckets[NBUCKETS];
} bucket_t;

typedef struct {
    bucket_t *keys;                               /* &hashtable entries */

    /* memory size in bytes/mb */
    size_t bytes;
    u32  mb;

    /* size in buckets/keys */
    size_t nbuckets;
    size_t nkeys;                                 /* nbuckets * NBUCKETS */

    /* internal representation */
    u32  nbits;                                   /* #buckets in bits, power of 2 */
    u32  mask;                                    /* nbuckets - 1, key mask */

    /* stats - unsure about usage */
    size_t used_buckets;
    size_t used_keys;
    u64 collisions;
} hasht_t;

/* hack:
 *  ep zobrist key index is 0-7 for each en-passant file, 8 for SQUARE_NONE.
 * To transform :
 *   - ep == 64 (SQUARE_NONE) to id = 8
 *   - ep == 0~63 to idx = sq_file(ep), i.e. (ep & 7)
 * we use the formula:
 * idx = ( ( ep & SQUARE_NONE ) >> 3 ) | sq_file(ep);
 */
#define EP_ZOBRIST_IDX(ep) ( ( (ep) >> 3 ) | sq_file(ep) )

extern hkey_t zobrist_pieces[16][64];
extern hkey_t zobrist_castling[4 * 4 + 1];
extern hkey_t zobrist_turn;                        /* for black, XOR each ply */
extern hkey_t zobrist_ep[9];                       /* 0-7: ep file, 8: SQUARE_NONE */

extern hasht_t hash_tt;                           /* main transposition table */

void zobrist_init(void);
hkey_t zobrist_calc(pos_t *pos);

#ifdef ZOBRIST_VERIFY
bool zobrist_verify(pos_t *pos);
#else
#define zobrist_verify(p) true
#endif

int hash_create(int Mb);
void hash_clear(void);
void hash_delete(void);

#endif  /* HASH_H */
