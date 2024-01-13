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
    export _BRCHESS_ENV_=1 BRCHESS_ROOT LD_LIBRARY_PATH
    BRCHESS_ROOT=$(realpath -L "$(dirname "${BASH_SOURCE[0]}")/..")

    BRCHESS_SCRIPTDIR="$BRCHESS_ROOT/scripts"
    BRCHESS_BINDIR="$BRCHESS_ROOT/bin"
    PATH="$PATH:$BRCHESS_BINDIR:$BRCHESS_SCRIPTDIR"

    BRCHESS_BRLIBDIR="$BRCHESS_ROOT/brlib"
    BRCHESS_LIBDIR="$BRCHESS_BRLIBDIR/lib"
    LD_LIBRARY_PATH="${BRCHESS_LIBDIR}${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
    #printf "R=%s L=%s LD=%s\n" "$BRCHESS_ROOT" "$BRLIB_DIR" "$LD_LIBRARY_PATH"
    unset BRCHESS_SCRIPTDIR BRCHESS_BINDIR BRCHESS_LIBDIR
    printf "brchess environment complete.\n"
fi
