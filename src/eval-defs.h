/* eval-defs.h - eval parameters
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

#ifndef EVAL_DEFS_H
#define EVAL_DEFS_H


#include "chessdefs.h"
#include "piece.h"
#include "eval.h"

/* game phases
 */
enum {
    MIDGAME,
    ENDGAME,
    PHASE_NB
};
typedef s16 phase_t;

/* pieces weight in phase calculation.
 */
enum {
    P_PHASE = 0,
    N_PHASE = 1,
    B_PHASE = 1,
    R_PHASE = 2,
    Q_PHASE = 4,
    ALL_PHASE = P_PHASE*16 + N_PHASE*4 + B_PHASE*4 + R_PHASE*4 + Q_PHASE*2
};

/* max pieces eval is 9*QUEEN_VALUE + 2*ROOK_VALUE + 2*BISHOP_VALUE
 * + 2*KNIGHT_VALUE which is (for a pawn valued at 100) well less than 15,000.
 */
#define EVAL_MAX     (SHRT_MAX)                   /* 32767 */
#define EVAL_MIN     (-EVAL_MAX)

#define EVAL_INV     EVAL_MIN

#define EVAL_MATE    30000

/* eval parameters */
enum {
    WT_MAT,
    WT_PST,

    TST_SPN,
    TST_CHK,

    EV_PARAMS_NB
};

enum {
    PAR_CHK,
    PAR_SPN,
    PAR_BTN,
    PAR_STR,
};

/**
 * ev_params - parameters definition
 * @init: eval_t default value
 * @min, @max: eval_t min and max values
 * @type: variable type (PARAM_CHECK, etc...)
 * @setable: bool setable (proposed in UCI options)
 * @name: char * human readable name
 */
struct ev_params {
    int type;
    bool setable;                                 /* true: proposed in UCI options */
    eval_t init, min, max;
    char *name;
};
void param_init(void);
int param_find_name(char *name);                  /* find parameter number by name */
char *param_name(const int num);

eval_t param_default(const int num);              /* get default param value */
eval_t param_min(const int num);
eval_t param_max(const int num);
bool param_setable(const int num);
int param_type(const int num);

/* parameters in use */
extern eval_t parameters[EV_PARAMS_NB];
#define param(n) parameters[n]
void param_set (int num, eval_t val);

/* PST data */
/**
 * pst_defs - pre-defined piece-square tables.
 */
enum {
    PST_ROFCHADE,
    PST_CPW,
    PST_SJENG,
    PST_NB
};

#define PST_DEFAULT PST_CPW
extern int pst_current;
extern eval_t pst_mg[COLOR_NB][PT_NB][SQUARE_NB];
extern eval_t pst_eg[COLOR_NB][PT_NB][SQUARE_NB];

void pst_set(char *str);
int pst_find(char *str);
void pst_init(int pst);

char *pst_name(int i);

#endif  /* EVAL_DEFS_H */
