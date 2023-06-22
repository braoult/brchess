# Makefile - GNU make only.
#
# Copyright (C) 2021-2023 Bruno Raoult ("br")
# Licensed under the GNU General Public License v3.0 or later.
# Some rights reserved. See COPYING.
#
# You should have received a copy of the GNU General Public License along with this
# program. If not, see <https://www.gnu.org/licenses/gpl-3.0-standalone.html>.
#
# SPDX-License-Identifier: GPL-3.0-or-later <https://spdx.org/licenses/GPL-3.0-or-later.html>
#

SHELL     := /bin/bash
CC        := gcc
BEAR      := bear
CCLSFILE  := compile_commands.json
DEPFILE   := make.deps

BINDIR    := ./bin
SRCDIR    := ./src
OBJDIR    := ./obj
INCDIR    := ./include
LIBDIR    := ./lib
LIBSRCDIR := ./libsrc
LDFLAGS   := -L$(LIBDIR)
LIB       := br_$(shell uname -m)
SLIB      := $(LIBDIR)/lib$(LIB).a
DLIB      := $(LIBDIR)/lib$(LIB).so
LIBSRC    := $(wildcard $(LIBSRCDIR)/*.c)
LIBOBJ    := $(patsubst %.c,%.o,$(LIBSRC))

SRC       := $(wildcard $(SRCDIR)/*.c)
INC       := $(wildcard $(SRCDIR)/*.h)
SRC_S     := $(notdir $(SRC))

.SECONDEXPANSION:
OBJ=$(addprefix $(OBJDIR)/,$(SRC_S:.c=.o))
BIN=fen piece move eval brchess

LIBS   = -l$(LIB) -lreadline -lncurses

CFLAGS += -std=gnu11

#CFLAGS += -O2
CFLAGS += -g
CFLAGS += -Wall
CFLAGS += -Wextra
CFLAGS += -march=native
CFLAGS += -Wmissing-declarations

##################################### DEBUG flags
CFLAGS += -DDEBUG		    # global
CFLAGS += -DDEBUG_DEBUG             # enable log() functions
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

##################################### General targets
.PHONY: compile cflags all clean

compile: lib $(OBJ) $(BIN)

cflags:
	@echo CFLAGS used: $(CFLAGS)

all: clean compile

clean:
	rm -rf $(OBJ) core $(BIN)

##################################### Generate and include dependencies
.PHONY: deps cleandeps $(DEPFILE)

cleandeps:
	rm -f $(DEPFILE)

deps: $(DEPFILE)

$(DEPFILE): $(SRC) $(INC)
	@echo generating dependancies.
	$(CC) -MM -MF $(DEPFILE) -I $(INCDIR) $(SRC)
	#cp $@ $@.sav
	sed -i "s|\(.*\.o\):|${OBJDIR}/\0:|" $@

include $(DEPFILE)

##################################### objects
.PHONY: obj

obj: $(OBJ)
#$(OBJ): $(OBJDIR)/%.o: $(SRCDIR)/%.c
#	@mkdir -p $(@D)
#	$(CC) -c $(CFLAGS) -o $@ $<
$(OBJDIR)/%.o: $(SRCDIR)/%.c
	echo SRC_S=$(SRC_S)
	echo O=$(OBJ)
	mkdir -p $(@D)
	@echo compiling A=$@ I=$<
	$(CC) -c $(CFLAGS) -I $(INCDIR) -o $@ $<

##################################### binaries
#fen: CFLAGS+=-DBIN_$$@
#$(BIN): $$(subst $(OBJDIR)/$$@.o,,$(OBJ)) $(SRCDIR)/$$@.c
#	@echo compiling $@.
#	@$(CC) -DBIN_$@ $(CFLAGS) $^ $(LIBS) -o $@

# TODO: find a better dependancy graph
$(BIN): $$(subst $(OBJDIR)/$$@.o,,$(OBJ)) $(SRCDIR)/$$@.c
	@echo compiling $@.
	@echo NEED_TO_CHANGE_THIS=$^
	$(CC) -DBIN_$@ $(CFLAGS) -I $(INCDIR) $^ $(LDFLAGS) $(LIBS) -o $@

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

##################################### br library
.PHONY: cleanlib lib

cleanlib: clean
	@$(RM) -f $(SLIB) $(DLIB) $(LIBOBJ)

lib: $(DLIB) $(SLIB)

$(SLIB): $(LIBOBJ)
	@echo building $@ static library.
	@mkdir -p $(LIBDIR)
	@$(AR) $(ARFLAGS) -o $@ $^

$(DLIB): CFLAGS += -fPIC
$(DLIB): LDFLAGS += -shared
$(DLIB): $(LIBOBJ)
	@echo building $@ shared library.
	@mkdir -p $(LIBDIR)
	@$(CC) $(LDFLAGS) $^ -o $@

.c.o:
	@echo compiling $<.
	@$(CC) -c $(CFLAGS) $(LDFLAGS) -I $(INCDIR) -o $@ $<
