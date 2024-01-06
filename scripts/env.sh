#!/usr/bin/env bash
#
# env.sh - set environment for brchess developer.
#
# Copyright (C) 2023 Bruno Raoult ("br")
# Licensed under the GNU General Public License v3.0 or later.
# Some rights reserved. See COPYING.
#
# You should have received a copy of the GNU General Public License along with this
# program. If not, see <https://www.gnu.org/licenses/gpl-3.0-standalone.html>.
#
# SPDX-License-Identifier: GPL-3.0-or-later <https://spdx.org/licenses/GPL-3.0-or-later.html>
#
# USAGE: source env.sh [arg]
#
# This file will actually be sourced if it was never sourced in current bash
# environment.

if [[ ! -v _BRCHESS_ENV_ ]]; then
    export _BRCHESS_ENV_=1 BRCHESS_ROOT BRLIBDIR LD_LIBRARY_PATH
    BRCHESS_ROOT=$(realpath -L "$(dirname "${BASH_SOURCE[0]}")/..")
    BRLIBDIR="$BRCHESS_ROOT/brlib/lib"
    LD_LIBRARY_PATH="${BRLIBDIR}${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
    PATH="$PATH:$BRCHESS_ROOT/bin:=$BRCHESS_ROOT/scripts"
    #printf "R=%s L=%s LD=%s\n" "$BRCHESS_ROOT" "$BRLIB_DIR" "$LD_LIBRARY_PATH"
    printf "Chess environment complete.\n"
    unset BRLIBDIR BRCHESS_ROOT
fi
