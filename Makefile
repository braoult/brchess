# Makefile - GNU make only.
#
# Copyright (C) 2021 Bruno Raoult ("br")
# Licensed under the GNU General Public License v3.0 or later.
# Some rights reserved. See COPYING.
#  * You should have received a copy of the GNU General Public License along with this
# program. If not, see <https://www.gnu.org/licenses/gpl-3.0-standalone.htmlL>.
#
# SPDX-License-Identifier: GPL-3.0-or-later <https://spdx.org/licenses/GPL-3.0-or-later.html>
#

BINDIR := ./bin
SRCDIR := ./src
OBJDIR := ./obj
DEPS   := make.deps

SRC=$(wildcard $(SRCDIR)/*.c)
INC=$(wildcard $(SRCDIR)/*.h)
SRC_S=$(notdir $(SRC))

CC=gcc

.SECONDEXPANSION:
OBJ=$(addprefix $(OBJDIR)/,$(SRC_S:.c=.o))
BIN=fen pool piece move debug eval bits bodichess

LIBS   = -lreadline -lncurses
CFLAGS += -std=gnu99

#CFLAGS += -O2
CFLAGS += -g
CFLAGS += -Wall
CFLAGS += -Wextra
CFLAGS += -march=native
#CFLAGS += -pedantic
#CFLAGS += -Wno-pointer-arith
#CFLAGS += -Werror
CFLAGS += -Wmissing-declarations

##################################### DEBUG flags
CFLAGS += -DDEBUG		    # global
CFLAGS += -DDEBUG_POOL              # memory pools management
CFLAGS += -DDEBUG_FEN               # FEN decoding
CFLAGS += -DDEBUG_MOVE              # move generation
CFLAGS += -DDEBUG_EVAL	            # eval functions
CFLAGS += -DDEBUG_PIECE             # piece list management
#CFLAGS += -DDEBUG_BITS	            # bits functions (take care !)

#CFLAGS += -DDEBUG_EVAL	            # sleep 1 sec within main loop (SIGINTR test)
#CFLAGS += -DDEBUG_EVAL2            # eval 2
#CFLAGS += -DDEBUG_EVAL3            # eval 3
#CFLAGS += -DDEBUG_MEM              # malloc

.PHONY: all cflags clean

compile: cflags $(OBJ) $(BIN)

cflags:
	@echo CFLAGS used: $(CFLAGS)

all: clean compile

$(DEPS): $(SRC) $(INC)
	@echo generating dependancies.
	@$(CC) -MM $(SRC) > $@
	@sed -i "s|\(.*\.o\):|${OBJDIR}/\0:|" $@

include $(DEPS)

clean:
	rm -rf $(OBJ) core $(BIN)

#$(OBJ): $(OBJDIR)/%.o: $(SRCDIR)/%.c
#	@mkdir -p $(@D)
#	$(CC) -c $(CFLAGS) -o $@ $<
$(OBJDIR)/%.o:
	@mkdir -p $(@D)
	@echo compiling $@.
	@$(CC) -c $(CFLAGS) -o $@ $<

#fen: CFLAGS+=-DBIN_$$@
#$(BIN): $$(subst $(OBJDIR)/$$@.o,,$(OBJ)) $(SRCDIR)/$$@.c
#	@echo compiling $@.
#	@$(CC) -DBIN_$@ $(CFLAGS) $^ $(LIBS) -o $@

# TODO: find a better dependancy graph
$(BIN): $$(subst $(OBJDIR)/$$@.o,,$(OBJ)) $(SRCDIR)/$$@.c
	@echo compiling $@.
	@echo NEED_TO_CHANGE_THIS=$^
	@$(CC) -DBIN_$@ $(CFLAGS) $^ $(LIBS) -o $@

#pool: CFLAGS+=-DPOOLBIN
#pool: $$(subst $(OBJDIR)/$$@.o,,$(OBJ)) $(SRCDIR)/$$@.c
#	$(CC) $(CFLAGS) $^ -o $@

# piece: CFLAGS+=-DPIECEBIN
# piece: $$(subst $(OBJDIR)/$$@.o,,$(OBJ)) $(SRCDIR)/$$@.c
# 	$(CC) $(CFLAGS) $^ -o $@

# move: CFLAGS+=-DMOVEBIN
# move: $$(subst $(OBJDIR)/$$@.o,,$(OBJ)) $(SRCDIR)/$$@.c
# 	$(CC) $(CFLAGS) $^ -o $@

# debug: CFLAGS+=-DDEBUGBIN
# debug: $$(subst $(OBJDIR)/$$@.o,,$(OBJ)) $(SRCDIR)/$$@.c
# 	$(CC) $(CFLAGS) $^ -o $@

#.PHONY: bits
#bits2: src/bits.c
#	$(CC) $(CFLAGS) -S $^ -o $@.s
#	$(CC) $(CFLAGS) $^ -o $@
