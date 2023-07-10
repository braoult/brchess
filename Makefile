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
TOUCH     := touch
RM        := rm
CCLSFILE  := compile_commands.json

SRCDIR    := ./src
INCDIR    := ./include
LIBSRCDIR := ./libsrc
BINDIR    := ./bin
OBJDIR    := ./obj
LIBDIR    := ./lib
LDFLAGS   := -L$(LIBDIR)

SRC       := $(wildcard $(SRCDIR)/*.c)                      # project sources
SRC_FN    := $(notdir $(SRC))                               # source basename

LIBSRC    := $(wildcard $(LIBSRCDIR)/*.c)                   # lib sources
LIBOBJ    := $(patsubst %.c,%.o,$(LIBSRC))                  # and objects

LIB       := br_$(shell uname -m)                           # library name
SLIB      := $(addsuffix .a, $(LIBDIR)/lib$(LIB))           # static lib
DLIB      := $(addsuffix .so, $(LIBDIR)/lib$(LIB))          # dynamic lib

BIN       := fen piece move eval brchess

LIBS      := -l$(LIB) -lreadline -lncurses

CFLAGS    := -std=gnu11

#CFLAGS += -O2
CFLAGS    += -g
CFLAGS    += -Wall
CFLAGS    += -Wextra
CFLAGS    += -march=native
CFLAGS    += -Wmissing-declarations
# for gprof
# CFLAGS += -pg
# Next one may be useful for valgrind (when invalid instructions)
# CFLAGS += -mno-tbm

##################################### DEBUG flags
CPPFLAGS  := -I$(INCDIR)
CPPFLAGS  += -DDEBUG                         # global
CPPFLAGS  += -DDEBUG_DEBUG                   # enable log() functions
#CPPFLAGS  += -DDEBUG_DEBUG_C                # enable verbose log() settings
CPPFLAGS  += -DDEBUG_POOL                    # memory pools management
CPPFLAGS  += -DDEBUG_FEN                     # FEN decoding
CPPFLAGS  += -DDEBUG_MOVE                    # move generation
CPPFLAGS  += -DDEBUG_EVAL                    # eval functions
CPPFLAGS  += -DDEBUG_PIECE                   # piece list management
CPPFLAGS  += -DDEBUG_SEARCH                  # move search

##################################### General targets
.PHONY: compile cflags all clean cleanall

compile: objects bin

cflags:
	@echo CFLAGS: "$(CFLAGS)"
	@echo CPPFLAGS: $(CPPFLAGS)
	@echo DEPFLAGS: $(DEPFLAGS)
	@echo LDFLAGS: $(LDFLAGS)

all: clean compile

clean: cleanobj cleanbin

cleanall: clean cleandeps cleanlib

##################################### Dependencies files
.PHONY: deps cleandeps

DEPDIR := ./.deps
DEPFILES := $(addprefix $(DEPDIR)/,$(SRC_FN:.c=.d))
DEPFLAGS = -MT $@ -MMD -MP -MF $(DEPDIR)/$*.d

$(DEPFILES):

include $(wildcard $(DEPFILES))

$(DEPDIR):
	@echo creating $@ directory.
	@mkdir -p $@

cleandeps:
	$(RM) -rf $(DEPDIR)

##################################### objects
.SECONDEXPANSION:
OBJ = $(addprefix $(OBJDIR)/,$(SRC_FN:.c=.o))

.PHONY: cleanobj

objects: $(OBJ)

cleanobj:
	$(RM) -rf $(OBJDIR)

$(OBJDIR):
	@echo creating $@ directory.
	@mkdir -p $@

$(OBJDIR)/%.o: $(SRCDIR)/%.c $(DEPDIR)/%.d | $(OBJDIR) $(DEPDIR)
	@echo compiling $<.
	@$(CC) -c $(CPPFLAGS) $(CFLAGS) $(DEPFLAGS) -I $(INCDIR) -o $@ $<

##################################### pre-processed (.i) and assembler (.s) output
%.i: %.c
	@echo generating $@
	@$(CC) -E $(CFLAGS) -I $(INCDIR) $< -o $@

%.s: %.c
	@echo generating $@
	@$(CC) -S -fverbose-asm $(CFLAGS) -I $(INCDIR) $< -o $@


##################################### br library
.PHONY: cleanlib libs

ARFLAGS = r

cleanlib:
	$(RM) -rf $(LIBDIR) $(LIBOBJ)

libs: $(DLIB) $(SLIB)

$(LIBDIR):
	@echo creating $@ directory.
	@mkdir -p $@

# remove default rule
%.o: %.c

$(LIBSRCDIR)/%.o: $(LIBSRCDIR)/%.c
	@echo compiling library $< "->" $@.
	@$(CC) -c $(CPPFLAGS) $(CFLAGS) -I $(INCDIR) -o $@ $<

$(SLIB): $(LIBOBJ) | $(LIBDIR)
	@echo building $@ static library.
	@$(AR) $(ARFLAGS) -o $@ $^

$(DLIB): CFLAGS += -fPIC
$(DLIB): LDFLAGS += -shared
$(DLIB): $(LIBOBJ) | $(LIBDIR)
	@echo building $@ shared library.
	@$(CC) $(LDFLAGS) $^ -o $@

##################################### binaries
.PHONY: bin prebin cleanbin

BINMARK := .bindone

bin: $(BIN) postbin

postbin:
	@[[ ! -f $(BINMARK) ]] || echo done.
	@$(RM) -f $(BINMARK)

cleanbin:
	$(RM) -f $(BIN) core

# TODO: find a better dependancy graph
$(BIN): $(SRCDIR)/$$@.c $(DLIB) $$(subst $(OBJDIR)/$$@.o,,$(OBJ))
	@[[ -f $(BINMARK) ]] || echo -n "generating binaries: "
	@echo -n "$@... "
	@$(CC) -DBIN_$@ $(CPPFLAGS) $(CFLAGS) -I $(INCDIR) $(subst libs,,$^) $(LDFLAGS) $(LIBS) -o $@
	@$(TOUCH) $(BINMARK)

##################################### ccls
.PHONY: ccls

ccls: $(CCLSFILE)

# generate compile_commands.json
$(CCLSFILE): brchess Makefile
	$(BEAR) -- make clean bin

##################################### LSP (ccls)
.PHONY: bear
ROOTDIR = .ccls-root

$(CCLSROOT):
	@echo creating root marker file.
	@$(TOUCH) $@

bear: clean $(CCLSROOT)
	@touch .ccls-root
	@$(BEAR) -- make compile

##################################### LSP (ccls)
.PHONY: memcheck

VALGRIND       = valgrind
VALGRINDFLAGS  = --leak-check=full --show-leak-kinds=all
VALGRINDFLAGS += --track-origins=yes --sigill-diagnostics=yes
VALGRINDFLAGS += --quiet --show-error-list=yes

memcheck: brchess
