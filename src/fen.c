/* fen.c - fen parsing/generation/test.
 *
 * Copyright (C) 2021-2024 Bruno Raoult ("br")
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
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

#include <debug.h>
#include <bug.h>

#include "chessdefs.h"
#include "misc.h"
#include "position.h"
#include "fen.h"

/* FEN description:
 * 1 : pieces on board (no space allowed):
 *   - rank 8 first, '/' between ranks
 *   - piece is usual piece notation(PNBRQK), black lowercase.
 *   - empty: number of consecutive empty squares (digit)
 * 2 : next move (w or b)
 * 3 : Castling capabilities: "-" if none, KQ/kq if white/black can castle
 *     on K or Q side
 * 4 : en-passant: "-" if none. If pawn just moved 2 squares, indicate target
 *     en-passant square (e.g. for e2-e4 this field is e3)
 * 5 : half moves since last capture or pawn advance (for 50 moves rule)
 * 6 : full moves, starts at 1, increments after black move
 *
 * Examples:
 *
 * starting position:
 * rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1
 * after 1.e4 e6 2.e5 d5
 * rnbqkbnr/ppp2ppp/4p3/3pP3/8/8/PPPP1PPP/RNBQKBNR w KQkq d6 0 3
 * 3.Nc3 Nc6 4.Rb1 Rb8 5.Nf3 h5 6.Be2
 * 1rbqkbnr/ppp2pp1/2n1p3/3pP2p/8/2N2N2/PPPPBPPP/1RBQK2R b Kk - 1 6
 * 6...Be7
 * 1rbqk1nr/ppp1bpp1/2n1p3/3pP2p/8/2N2N2/PPPPBPPP/1RBQK2R w Kk - 2 7
 * 7.Nxd5 h4 8.g4
 * 1rbqk1nr/ppp1bpp1/2n1p3/3NP3/6Pp/5N2/PPPPBP1P/1RBQK2R b Kk g3 0 8
 */

/*  chess startup position FEN */
const char *startfen="rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
static const char *castle_str = "KQkq";

#define SKIP_BLANK(p) for(;isspace(*(p)); (p)++)

/**
 * fen_ok() - test (and try to fix) fen-generated position.
 * @pos:    &position
 * @fixit:  action flag
 *
 * Test and possibly fix the following:
 * - inconsistent castle flags (if K & R are not in correct position)
 * - inconsistent en-passant square (turn, bad pawns positions)
 *
 * if @fixit is true, any error above will be fixed, and pos_ok() will
 is also called, leading to fatal errors if something is wrong.
 *
 * @return: 0 if OK, 1 if OK after fix, -1 if fatal issue.
 */
int fen_ok(pos_t *pos, bool fixit)
{
    char *colstr[2] = { "white", "black"};
    int warning = 0;
    color_t us = pos->turn, them = OPPONENT(us);
    int up = sq_up(us);                           /* pawn up */
    square_t ep = pos->en_passant;

    /* en passant, depends on who plays next */
    if (ep != SQUARE_NONE) {
        rank_t eprank = sq_rank(ep);
        rank_t rank6 = sq_rel_rank(RANK_6, us);
        piece_t pawn = MAKE_PIECE(PAWN, them);
        bitboard_t att = bb_pawn_attacks[them][ep] & pos->bb[us][PAWN];

        if (eprank != rank6 ||
            pos->board[ep - up] != pawn ||
            pos->board[ep] != EMPTY ||
            pos->board[ep + up] != EMPTY ||
            att == 0ull) {

            warning++;
            if (fixit) {
                warn(true, "pos warn: wrong en-passant settings (fixed).\n");
                pos->en_passant = SQUARE_NONE;
            }
        }
    }

    for (int color = WHITE; color <= BLACK; ++color) {
        rank_t rank1 = sq_rel_rank(RANK_1, color);

        /* castling */
        /* where K and R should be for valid castle flag */
        bitboard_t k    = bb_sq[sq_make(FILE_E, rank1)];
        bitboard_t r_k  = bb_sq[sq_make(FILE_H, rank1)];
        bitboard_t r_q  = bb_sq[sq_make(FILE_A, rank1)];

       /* where they are */
        bitboard_t kingbb = pos->bb[color][KING];
        bitboard_t rookbb = pos->bb[color][ROOK];
        castle_rights_t castle_k = color == WHITE? CASTLE_WK: CASTLE_BK;
        castle_rights_t castle_q = color == WHITE? CASTLE_WQ: CASTLE_BQ;
        if (pos->castle & castle_k && !(k & kingbb && r_k & rookbb)) {
            warning++;
            if (fixit) {
                warn(true, "fen warn: wrong %s short castling (fixed)\n",
                     colstr[color]);
                pos->castle &= ~castle_k;
            }
        }
        if (pos->castle & castle_q && !(k & kingbb && r_q & rookbb)) {
            warning++;
            if (fixit) {
                warn(true, "fen warn: wrong %s long castling (fixed)\n",
                     colstr[color]);
                pos->castle &= ~castle_q;
            }
        }
    }
    return warning;
}

/**
 * startpos - create a game start position
 * @pos: a position pointer or NULL
 *
 * See @fen2pos function.
 *
 * @return: the pos position.
 */
pos_t *startpos(pos_t *pos)
{
    return fen2pos(pos, startfen);
}

/**
 * fen2pos - make a position from a fen string
 * @pos: a position pointer or NULL
 * @fen: a valid fen string
 *
 * If @pos is NULL, a position will be allocated with @pos_new(),
 * that should be freed by caller.
 *
 * @return: the pos position, or NULL if error.
 */
pos_t *fen2pos(pos_t *pos, const char *fen)
{
    const char *cur = fen;
    char *p;
    short rank, file, tmp;
    piece_t piece;
    int consumed, err_line = 0, err_pos = 0, err_char = 0;
    pos_t tmppos;

    pos_clear(&tmppos);
    /* 1) get piece placement information
     */
    for (rank = 7, file = 0; *cur && !isspace(*cur); ++cur) {
        if (*cur == '/') {                        /* next rank */
            rank--;
            file = 0;
            continue;
        }
        if (isdigit(*cur)) {                      /* empty square(s) */
            file += *cur - '0';
            continue;
        }
        if ((piece = piece_from_fen(*cur)) != EMPTY) {
#           ifdef DEBUG_FEN
            printf("f=%d r=%d *p=%c piece=%#04x t=%d c=%d\n", file, rank, *cur,
                  piece, PIECE(piece), COLOR(piece));
#           endif
            pos_set_sq(&tmppos, sq_make(file, rank), piece);
            if (PIECE(piece) == KING)
                tmppos.king[COLOR(piece)] = sq_make(file, rank);
            file++;
        } else {                                  /* error */
            err_line = __LINE__, err_char = *cur, err_pos = cur - fen;
            goto end;
        }
    }
    SKIP_BLANK(cur);

    /* 2) next turn color
     */
    tmppos.turn = *cur++ == 'w' ? WHITE : BLACK;
    SKIP_BLANK(cur);

    /* 3) castle rights
     */
    if (*cur == '-') {
        cur++;
    } else {
        for (; *cur && !isspace(*cur); ++cur) {
            if ((p = strchr(castle_str, *cur))) { /* valid castle letter */
                tmppos.castle |= 1 << (p - castle_str);
            } else {
                err_line = __LINE__, err_char = *cur, err_pos = cur - fen;
                goto end;
            }
        }
    }
    SKIP_BLANK(cur);

    /* 4) en passant
     */
    if (*cur == '-') {
        cur++;
    } else {
        tmppos.en_passant = sq_from_string(cur);
        cur += 2;
    }
    SKIP_BLANK(cur);

    /* 5) half moves since last capture or pawn move (50 moves rule)
     */
    tmppos.clock_50 = 0;
    tmppos.plycount = 1;
    if (sscanf(cur, "%hd%n", &tmp, &consumed) != 1)
        goto end;                                 /* early end, ignore w/o err */

    tmppos.clock_50 = tmp;
    cur += consumed;
    SKIP_BLANK(cur);

    /* 6) current full move number, starting with 1
     */
    if (sscanf(cur, "%hd", &tmp) != 1)
        goto end;
    if (tmp <= 0)                                 /* fix faulty numbers*/
        tmp = 1;
    tmp = 2 * (tmp - 1) + (tmppos.turn == BLACK); /* plies, +1 if black turn */
    tmppos.plycount = tmp;

end:
    if (warn(err_line, "FEN error line %d: charpos=%d char=%#x(%c)\n",
             err_line, err_pos, err_char, err_char)) {
        return NULL;
    }

    fen_ok(&tmppos, true);                        /* fix e.p & castling flags */
    if (!pos_ok(&tmppos, false))
        return NULL;                              /* invalid position: ignored */

    tmppos.key = zobrist_calc(&tmppos);
    if (!pos)
        pos = pos_new();
    pos_copy(&tmppos, pos);
#   ifdef DEBUG_FEN
    pos_print_raw(&tmppos, 1);
#   endif

    return pos;
}

/**
 * pos2fen - make a FEN string from a position.
 * @pos: a position pointer
 * @fen: destination FEN char*, or NULL
 *
 * If @fen is NULL, a 92 bytes memory will be allocated with malloc(1),
 * that should be freed by caller.
 *
 * Note: If @fen is given, no check is done on its length, but to
 * be on secure side, it should be at least 90 bytes. See:
 * https://chess.stackexchange.com/questions/30004/longest-possible-fen
 * For convenience, use FENSTRLEN.
 *
 * @return: the pos position, or NULL if error.
 */
char *pos2fen(const pos_t *pos, char *fen)
{
    int cur = 0;

    if (!fen)
        fen = safe_malloc(92);

    /*  1) position
     */
    for (rank_t r = RANK_8; r >= RANK_1; --r) {
        for (file_t f = FILE_A; f <= FILE_H;) {
            square_t sq = sq_make(f, r);
            piece_t piece = pos->board[sq];
#           ifdef DEBUG_FEN
            printf("r=%d f=%d p=%d pos=%d\n", r, f, piece, cur);
#           endif
            if (piece == EMPTY) {
                int len = 0;
                for (; f <= FILE_H && pos->board[sq_make(f, r)] == EMPTY; f++)
                    len++;
#               ifdef DEBUG_FEN
                printf("empty=%d char=%c\n", len, '0' + len);
#               endif
                fen[cur++] = '0' + len;
            } else {
                fen[cur++] = *piece_to_fen(piece);
#               ifdef DEBUG_FEN
                printf("f1=%d r=%d c=%c t=%d c=%d \n", f, r,
                       *(piece_to_fen(piece)), PIECE(piece), COLOR(piece));
#               endif
                f++;
            }
        }
        fen[cur++] = r == RANK_1? ' ': '/';
    }

    /* 2) next turn color
     */
    fen[cur++] = pos->turn == WHITE? 'w': 'b';
    fen[cur++] = ' ';

    /*  3) castle rights
     */
    if (pos->castle == 0) {
        fen[cur++] = '-';
    } else {
        for (int i = 0; i < 4; ++i)
            if (pos->castle & BIT(i))
                fen[cur++] = castle_str[i];
    }
    fen[cur++] = ' ';

    /* 4) en passant
     */
    if (pos->en_passant == SQUARE_NONE) {
        fen[cur++] = '-';
    } else {
        strcpy(fen+cur, sq_to_string(pos->en_passant));
        cur += 2;
    }
    fen[cur++] = ' ';

    /* 5) moves since last capture or pawn move (50 moves rule)
     * 6) current full move number, starting with 1
     */
    sprintf(fen+cur, "%d %d", pos->clock_50,
            1 + (pos->plycount - (pos->turn == BLACK)) / 2);
    return fen;
}
