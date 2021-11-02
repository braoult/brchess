/* move.h - debug/log management.
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

#ifndef DEBUG_H
#define DEBUG_H

#include <stdbool.h>
#include <stdint.h>

#ifdef DEBUG

void debug_init(void);

void debug(bool timestamp, uint32_t indent,
           const char *src, uint32_t line, const char *, ...);

/* format: func name, no indent, no timestamp
 * foo:15 val=2
 */
#define log(fmt, args...) debug(false, 0, __func__, __LINE__, fmt, ##args)

/* format : func name, indent, no timestamp
 *   foo:15 val=2
 */
#define log_i(i, fmt, args...) debug(false, (i), __func__, __LINE__, fmt, args)

/* format : func name, indent, timestamp
 *   []foo:15 val=2
 */
#define log_it(i, fmt, args...) debug(true, (i), __func__, __LINE__, fmt, args)

/* format: file name, no indent, no timestamp
 * foo:15 val=2
 */
#define log_f(fmt, args...) debug(false, 0, __FILE__, __LINE__, fmt, args)



#else
#define log(...)
#define f
#endif  /* DEBUG */

#endif  /* DEBUG_H */
