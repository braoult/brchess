/* fen.c - fen notation.
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

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

#include "debug.h"
#include "chessdefs.h"
#include "position.h"
#include "board.h"
#include "fen.h"
#include "piece.h"

/* Starting Position :
 * rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1
 * After 1.e4 :
 * rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1
 * After 1... c5 :
 * rnbqkbnr/pp1ppppp/8/2p5/4P3/8/PPPP1PPP/RNBQKBNR w KQkq c6 0 2
 * After 2. Nf3:
 * rnbqkbnr/pp1ppppp/8/2p5/4P3/5N2/PPPP1PPP/RNBQKB1R b KQkq - 1 2
 *
 * 1 : White uppercase
 * 2 : next move (w or b)
 * 3 : Castling capabilities: "-" if none, KQ/kq if white/black can castle
 *     on K or Q side
 * 4 : en-passant: if pawn just moved 2 squares, indicate target square (e.g.
 *     for e2-e4 this field is e3)
 * 5 : half moves since last capture or pawn advance (for 50 moves rule)
 * 6 : full moves, starts at 1, increments after black move
 *
 */

// warning, we expect a valid fen input
pos_t *fen2pos(pos_t *pos, char *fen)
{
    char *p = fen;
    short rank, file, skip, color;
    piece_t piece;
    board_t *board = pos->board;
#   define SKIP_BLANK(p) for(;*(p) == ' '; (p)++)

    /* 1) get piece placement information
     */
    for (rank = 7, file = 0; *p && *p != ' '; ++p) {
        color = isupper(*p)? WHITE: BLACK;
        char cp = toupper(*p);
        switch (cp) {
            case CHAR_PAWN:
                piece = PAWN;
                goto set_square;
            case CHAR_KNIGHT:
                piece = KNIGHT;
                goto set_square;
            case CHAR_BISHOP:
                piece = BISHOP;
                goto set_square;
            case CHAR_ROOK:
                piece = ROOK;
                goto set_square;
            case CHAR_QUEEN:
                piece = QUEEN;
                goto set_square;
            case CHAR_KING:
                piece = KING;
                pos->king[color]=SQ88(file, rank);
                //goto set_square;
            set_square:
#               ifdef DEBUG_FEN
                log_i(5, "f=%d r=%d *p=%c piece=%c color=%d\n",
                       file, rank, *p, cp, color);
#               endif
                pos->occupied[color] |= BB(file, rank);
                SET_COLOR(piece, color);
                board[SQ88(file, rank)].piece = piece;
                board[SQ88(file, rank)].s_piece =
                    piece_add(pos, piece, SQUARE(file, rank));
                file++;
                break;
            case '/':
                rank--;
                file = 0;
                break;
            default:
                skip = cp - '0';
                while (skip--) {
                    board[SQ88(file++, rank)].piece = EMPTY;
                }
        }
    }
#   ifdef DEBUG_FEN
    for (rank = 7; rank >= 0; --rank) {
        for (file = 0; file < 8; ++file) {
            log(5, "%02x ", board[SQ88(file, rank)].piece);
        }
        log(5, "\n");
    }
#   endif

    /* 2) next move color
     */
    SKIP_BLANK(p);
    SET_COLOR(pos->turn, *p == 'w' ? WHITE : BLACK);
    p++;

    /* 3) castle status
     */
    SKIP_BLANK(p);
    pos->castle = 0;
    if (*p != '-') {
        for (; *p && *p != ' '; ++p) {
            switch (*p) {
                case 'K':
                    pos->castle |= CASTLE_WK;
                    break;
                case 'k':
                    pos->castle |= CASTLE_BK;
                    break;
                case 'Q':
                    pos->castle |= CASTLE_WQ;
                    break;
                case 'q':
                    pos->castle |= CASTLE_BQ;
                    break;
            }
        }
    }
    p++;

    /* 4) en passant
     */
    SKIP_BLANK(p);
    pos->en_passant = 0;
    if (*p != '-') {
        SET_F(pos->en_passant, C2FILE(*p++));
        SET_R(pos->en_passant, C2RANK(*p++));
    } else {
        p++;
    }

    /* 5) half moves since last capture or pawn move and
     * 6) current move number
     */
    SKIP_BLANK(p);
    log_i(5, "pos=%d\n", (int)(p-fen));
    sscanf(p, "%hd %hd", &pos->clock_50, &pos->curmove);
    return pos;
}

#ifdef BIN_fen
int main(int ac, char**av)
{
    pos_t *pos;

    debug_init(5);
    piece_pool_init();
    pos = pos_create();
    if (ac == 1) {
        pos_startpos(pos);
    } else {
        fen2pos(pos, av[1]);
    }
    pos_print(pos);
}
#endif
