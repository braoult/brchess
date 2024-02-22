/* fen.c - fen notation.
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
#include "util.h"
#include "piece.h"
#include "bitboard.h"
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
 * fen_test(pos_t *pos) - test (and try to fix) fen-generated position.
 * @pos: position
 *
 * fen_test() tests the following:
 * - fatal: number of pawns > 8
 * - fatal: number of pieces > 16
 * - fatal: number of kings != 1
 * - fixable: inconsistent castle flags (if K & R are not in correct position)
 * - fixable: inconsistent en-passant square (turn, bad pawn position)
 *
 * @return: 0 if OK, 1 if OK after fix, -1 if fatal issue.
 */
static int fen_test(pos_t *pos)
{
    char *colstr[2] = { "white", "black"};
    int error = 0, warning = 0;

    /* en passant, depends on who plays next */
    if (pos->en_passant != SQUARE_NONE) {
        rank_t eprank = sq_rank(pos->en_passant);
        file_t epfile = sq_file(pos->en_passant);
        rank_t rank5 = pos->turn == WHITE? RANK_5: RANK_4;
        rank_t rank6 = pos->turn == WHITE? RANK_6: RANK_3;
        rank_t rank7 = pos->turn == WHITE? RANK_7: RANK_2;
        piece_t pawn = pos->turn == WHITE? B_PAWN: W_PAWN;
        if (warn(eprank != rank6 ||
                 pos->board[sq_make(epfile, rank5)] != pawn ||
                 pos->board[sq_make(epfile, rank6)] != EMPTY ||
                 pos->board[sq_make(epfile, rank7)] != EMPTY,
                 "fen warn: wrong en-passant settings. (fixed)\n")) {
            printf("ep5=%o ep6=%o ep7=%o\n", sq_make(epfile, rank5),
                   sq_make(epfile, rank6), sq_make(epfile, rank6));
            warning++;
            pos->en_passant = SQUARE_NONE;
        }
    }

    for (int color = WHITE; color <= BLACK; ++color) {
        int n;
        rank_t rank1 = color == WHITE? RANK_1: RANK_8;

        /* castling */
        /* where K and R should be for valid castle flag */
        bitboard_t k    = bb_sq[sq_make(FILE_E, rank1)];
        bitboard_t r_k  = bb_sq[sq_make(FILE_H, rank1)];
        bitboard_t r_q  = bb_sq[sq_make(FILE_A, rank1)];

       /* where they are */
        bitboard_t kings = pos->bb[color][KING];
        bitboard_t rooks = pos->bb[color][ROOK];
        castle_rights_t castle_k = color == WHITE? CASTLE_WK: CASTLE_BK;
        castle_rights_t castle_q = color == WHITE? CASTLE_WQ: CASTLE_BQ;
        if (pos->castle & castle_k) {
            if (warn(!(k & kings && r_k & rooks),
                     "fen warn: wrong %s short castling K or R position (fixed)\n",
                     colstr[color])) {
                warning++;
                pos->castle &= ~castle_k;
            }
        }
        if (pos->castle & castle_q) {
            if (warn(!(k & kings && r_q & rooks),
                     "fen warn: wrong %s long castling K or R position (fixed)\n",
                     colstr[color])) {
                warning++;
                pos->castle &= ~castle_q;
            }
        }

        /* piece, pawn, anf king count */
        n = popcount64(pos->bb[color][PAWN]);
        error += warn(n > 8,
                      "fen err: %s has %d pawns\n", colstr[color], n);
        n = popcount64(pos->bb[color][KING]);
        error += warn(n != 1,
                      "fen err: %s has %d kings\n", colstr[color], n);
        n = popcount64(pos->bb[color][ALL_PIECES]);
        error += warn(n > 16,
                      "fen err: %s has %d pieces\n", colstr[color], n);
    }
    return error ? -1: warning ? 1: 0;
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
    int consumed, err_line = 0, err_pos, err_char;
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
        if ((piece = char_color_to_piece(*cur)) != EMPTY) {
#           ifdef DEBUG_FEN
            log_i(5, "f=%d r=%d *p=%c piece=%#04x t=%d c=%d\n", file, rank, *cur,
                  piece, PIECE(piece), COLOR(piece));
#           endif
            pos_set_sq(&tmppos, sq_make(file, rank), piece);
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
        tmppos.en_passant = sq_make(C2FILE(*cur), C2RANK(*(cur+1)));
        cur += 2;
    }
    SKIP_BLANK(cur);

    /* 5) half moves since last capture or pawn move (50 moves rule)
     */
    sscanf(cur, "%hd%n", &tmppos.clock_50, &consumed);
    cur += consumed;
    SKIP_BLANK(cur);

    /* 6) current full move number, starting with 1
     */
    sscanf(cur, "%hd", &tmp);
    if (tmp <= 0)                                 /* fix faulty numbers*/
        tmp = 1;
    tmp = 2 * (tmp - 1) + (tmppos.turn == BLACK); /* plies, +1 if black turn */

    tmppos.plycount = tmp;

#   ifdef DEBUG_FEN
    raw_board_print(&tmppos);
#   endif

end:
    if (warn(err_line, "FEN error line %d: charpos=%d char=%#x(%c)\n",
             err_line, err_pos, err_char, err_char)) {
        return NULL;
    }
    if (fen_test(&tmppos) >= 0) {
        if (!pos)
            pos = pos_new();
        *pos = tmppos;
    }
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
            if (pos->board[sq] == EMPTY) {
                int len = 0;
                for (; f <= FILE_H && pos->board[sq_make(f, r)] == EMPTY; f++)
                    len++;
                fen[cur++] = '0' + len;
            } else {
                fen[cur++] = piece_to_char_color(piece);
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
            if (pos->castle & mask(i))
                fen[cur++] = castle_str[i];
    }
    fen[cur++] = ' ';

    /* 4) en passant
     */
    if (pos->en_passant == SQUARE_NONE) {
        fen[cur++] = '-';
    } else {
        fen[cur++] = FILE2C(sq_file(pos->en_passant));
        fen[cur++] = RANK2C(sq_rank(pos->en_passant));
    }
    fen[cur++] = ' ';

    /* 5) moves since last capture or pawn move (50 moves rule)
     * 6) current full move number, starting with 1
     */
    sprintf(fen+cur, "%d %d", pos->clock_50,
            1 + (pos->plycount - (pos->turn == BLACK)) / 2);
    return fen;
}
