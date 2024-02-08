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
#include "position.h"
#include "fen.h"
#include "piece.h"
#include "util.h"

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

/* next must follow 'piece_type' enum values (pawn is 1, king is 6) */
static const char *pieces_str = " PNBRQK";
/* And this one must follow 'castle' enum order (also same as usual FEN) */
static const char *castle_str = "KQkq";

#define SKIP_BLANK(p) for(;isspace(*(p)); (p)++)

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
 * If @pos is NULL, a position will be allocated with malloc(1),
 * that should be freed by caller.
 *
 * @return: the pos position, or NULL if error.
 */
pos_t *fen2pos(pos_t *pos, const char *fen)
{
    const char *cur = fen;
    char *p;
    short rank, file, color, tmp;
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
        color = isupper(*cur)? WHITE: BLACK;
        if ((p = strchr(pieces_str, toupper(*cur)))) {  /* valid piece letter */
#           ifdef DEBUG_FEN
            log_i(5, "f=%d r=%d *p=%c piece=%c color=%d ppos=%ld\n",
                  file, rank, *cur, *p, color, p - pieces_str);
#           endif
            pos_set_sq(&tmppos, p - pieces_str, color, file, rank);
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
    tmppos.en_passant = 0;
    if (*cur == '-') {
        cur++;
    } else {
        tmppos.en_passant = BB(C2FILE(*cur), C2RANK(*(cur+1)));
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
    printf ("remain=[%s]\n", cur);
    sscanf(cur, "%hd", &tmp);
    printf ("tmp=[%d]\n", tmp);
    if (tmp <= 0)                                 /* fix faulty numbers*/
        tmp = 1;
    printf ("tmp2=[%d]\n", tmp);
    tmp = 2 * (tmp - 1) + (tmppos.turn == BLACK); /* plies, +1 if black turn */
    printf ("tmp3=[%d]\n", tmp);

    tmppos.plycount = tmp;

#   ifdef DEBUG_FEN
    for (rank = 7; rank >= 0; --rank) {
        for (file = 0; file < 8; ++file) {
            log(5, "%02x ", tmppos.board[BB(file, rank)]);
        }
        log(5, "\n");
    }
    log(5, "turn=%d 50_rule=%d curply=%d\n",
        tmppos.turn, tmppos.clock_50, tmppos.plycount);
#   endif

end:
    if (warn(err_line, "FEN error line %d: pos=%d char=%#x(%c)\n",
             err_line, err_pos, err_char, err_char)) {
        return NULL;
    }
    if (!pos)
        pos = pos_new();
    else
        pos_clear(pos);
    *pos = tmppos;
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
    for (rank_t rank = RANK_8; rank >= RANK_1; --rank) {
        printf("r=%d 1=%d\n", rank, RANK_1);
        for (file_t file = FILE_A; file <= FILE_H;) {
            printf(" f=%d H=%d\n", file, FILE_H);
            square_t sq = BB(file, rank);
            printf("  sq=%d\n", sq);
            piece_t piece = PIECE(pos->board[sq]);
            color_t color = COLOR(pos->board[sq]);
            if (pos->board[sq] == EMPTY) {
                int len = 0;
                for (; file <= FILE_H && pos->board[BB(file,rank)] == EMPTY; file++)
                    len++;
                fen[cur++] = '0' + len;
            } else {
                char c = pieces_str[piece];
                fen[cur++] = color == WHITE? c: tolower(c);
                file++;
            }
            fen[cur]=0; puts(fen);
        }
        fen[cur++] = rank == RANK_1? ' ': '/';
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
    if (!pos->en_passant) {
        fen[cur++] = '-';
    } else {
        fen[cur++] = FILE2C(BBfile(pos->en_passant));
        fen[cur++] = RANK2C(BBrank(pos->en_passant));
    }
    fen[cur++] = ' ';

    /* 5) moves since last capture or pawn move (50 moves rule)
     * 6) current full move number, starting with 1
     */
    sprintf(fen+cur, "%d %d", pos->clock_50,
            1 + (pos->plycount - (pos->turn == BLACK)) / 2);
    return fen;
}
