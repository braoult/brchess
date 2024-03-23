# Makefile - brchess Makefile, **GNU make only**
#
# Copyright (C) 2021-2024 Bruno Raoult ("br")
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
#CC        := clang
LD        := ld
BEAR      := bear
TOUCH     := touch
RM        := rm
RMDIR     := rmdir
MAKE      := make

SRCDIR    := ./src
INCDIR    := ./src					    # used by ./test sources
OBJDIR    := ./obj
BINDIR    := ./bin
DEPDIR    := ./dep
TSTDIR    := ./test

BRLIB     := ./brlib
BRINCDIR  := $(BRLIB)/include
BRLIBDIR  := $(BRLIB)/lib

CCLSROOT  := .ccls-root
CCLSFILE  := compile_commands.json

SRC       := $(wildcard $(SRCDIR)/*.c)			    # project sources
SRC_FN    := $(notdir $(SRC))				    # source basename
OBJ       := $(addprefix $(OBJDIR)/,$(SRC_FN:.c=.o))

TSTSRC    := $(wildcard $(TSTDIR)/*.c)

LIB       := br_$(shell uname -m)			    # library name
LIBS      := $(strip -l$(LIB) -lreadline)

DEP_FN    := $(SRC_FN)
DEP       := $(addprefix $(DEPDIR)/,$(DEP_FN:.c=.d))

TARGET_FN := brchess
TARGET    := $(addprefix $(BINDIR)/,$(TARGET_FN))

ASMFILES  := $(SRC:.c=.s) $(TSTSRC:.c=.s)
CPPFILES  := $(SRC:.c=.i) $(TSTSRC:.c=.i)

##################################### pre-processor flags
CPPFLAGS  := -I$(BRINCDIR) -I$(INCDIR)

CPPFLAGS  += -DNDEBUG                                       # assert

CPPFLAGS  += -DBUG_ON                                       # brlib bug.h
CPPFLAGS  += -DWARN_ON                                      # brlib bug.h

#CPPFLAGS  += -DDEBUG                                        # global - unused
#CPPFLAGS  += -DDEBUG_DEBUG                                  # enable log() functions
#CPPFLAGS  += -DDEBUG_DEBUG_C                                # enable log() settings
#CPPFLAGS  += -DDEBUG_POOL                                   # memory pools management
#CPPFLAGS  += -DDEBUG_POS				     # position.c
#CPPFLAGS  += -DDEBUG_MOVE                                   # move generation

# fen.c
#CPPFLAGS  += -DDEBUG_FEN                                    # FEN decoding

# attack.c
#CPPFLAGS  += -DDEBUG_ATTACK_ATTACKERS1			     # sq_attackers details
CPPFLAGS  += -DDEBUG_ATTACK_ATTACKERS			    # sq_attackers
CPPFLAGS  += -DDEBUG_ATTACK_PINNERS			    # sq_pinners details

#CPPFLAGS  += -DDEBUG_EVAL                                   # eval functions
#CPPFLAGS  += -DDEBUG_PIECE                                  # piece list management
#CPPFLAGS  += -DDEBUG_SEARCH                                 # move search

CPPFLAGS  += -DDIAGRAM_SYM                                  # diagram with symbols

# remove extraneous spaces (due to spaces before comments)
CPPFLAGS  := $(strip $(CPPFLAGS))

##################################### compiler flags
CFLAGS    := -std=gnu11
#CFLAGS    += -flto
CFLAGS    += -O1
CFLAGS    += -g
CFLAGS    += -Wall
CFLAGS    += -Wextra
CFLAGS    += -march=native
CFLAGS    += -Wmissing-declarations
# for gprof
# CFLAGS += -pg
# Next one may be useful for valgrind (when invalid instructions)
# CFLAGS += -mno-tbm

CFLAGS    := $(strip $(CFLAGS))

##################################### linker flags
LDFLAGS   := -L$(BRLIBDIR)
#LDFLAGS   += -flto

LDFLAGS   := $(strip $(LDFLAGS))

##################################### archiver/dependency flags
ARFLAGS   := rcs
DEPFLAGS   = -MMD -MP -MF $(DEPDIR)/$*.d

##################################### archiver/dependency flags
ALL_CFLAGS  = $(DEPFLAGS) $(CPPFLAGS) $(CFLAGS)
ALL_LDFLAGS = $(LDFLAGS) $(LIBS)

##################################### General targets
.PHONY: all compile clean cleanall

all: $(TARGET)

compile: brlib objs

libs: brlib

clean: cleandep cleanasmcpp cleanobj cleanbin

cleanall: clean cleandepdir cleanobjdir cleanallbrlib cleanbindir

##################################### cleaning functions
# rmfiles - delete a list of files if they exist.
# $(1): the directory
# $(2): The string to include in action output - "cleaning X files."
# see: https://stackoverflow.com/questions/6783243/functions-in-makefiles
#
# Don't use wildcard like "$(DIR)/*.o", so we can control mismatches between
# list and actual files in directory.
# See rmdir below.
define rmfiles
	@#echo "rmfiles=+$(1)+"
	$(eval $@_EXIST = $(wildcard $(1)))
	@#echo "existfile=+${$@_EXIST}+"
	@if [[ -n "${$@_EXIST}" ]]; then         \
		echo "cleaning $(2) files." ;    \
		$(RM) ${$@_EXIST} ;              \
	fi
endef

# rmdir - delete a directory if it exists.
# $(1): the directory
# $(2): The string to include in action output - "removing X dir."
#
# Don't use $(RM) -rf, to control unexpected dep files.
# See rmfile above.
define rmdir
	@#echo "rmdir +$(1)+"
	$(eval $@_EXIST = $(wildcard $(1)))
	@#echo "existdir=+${$@_EXIST}+"
	@if [[ -n "${$@_EXIST}" ]]; then         \
		echo "removing $(2) dir." ;      \
		$(RMDIR) ${$@_EXIST} ;           \
	fi
endef

##################################### dirs creation
.PHONY: alldirs

ALLDIRS   := $(DEPDIR) $(OBJDIR) $(BINDIR)

alldirs: $(ALLDIRS)

# Here, we have something like:
# a: a
# a will be built if (1) older than a, or (2) does not exist. Here only (2).
$(ALLDIRS): $@
	@echo creating $@ directory.
	@mkdir -p $@

##################################### Dependencies files
.PHONY: cleandep cleandepdir

-include $(wildcard $(DEP))

# Don't use $(DEPDIR)/*.d, to control mismatches between dep and src files.
# See second rule below.
cleandep:
	$(call rmfiles,$(DEP),depend)
	@#echo cleaning dependency files.
	@#$(RM) -f $(DEP)

cleandepdir:
	$(call rmdir,$(DEPDIR),depend)
	@#[ -d $(DEPDIR) ] && echo cleaning depend files && $(RM) -f $(DEP) || true

##################################### brchess objects
.PHONY: objs cleanobj cleanobjdir

objs: $(OBJ)

cleanobj:
	$(call rmfiles,$(OBJ),object)

cleanobjdir: cleanobj
	$(call rmdir,$(OBJDIR),objects)

# The part right of '|' are "order-only prerequisites": They are build as
# "normal" ones, but do not imply to rebuild target.
$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR) $(DEPDIR)
	@echo compiling brchess module: $< "->" $@.
	@$(CC) -c $(ALL_CFLAGS) $< -o $@

##################################### brlib libraries
.PHONY: cleanbrlib cleanallbrlib brlib

cleanbrlib:
	$(MAKE) -C $(BRLIB) clean

cleanallbrlib:
	$(MAKE) -C $(BRLIB) cleanall

brlib:
	$(MAKE) -C $(BRLIB) libs

##################################### brchess binaries
.PHONY: targets cleanbin cleanbindir

targets: $(TARGET)

cleanbin:
	$(call rmfiles,$(TARGET),binary)

cleanbindir:
	$(call rmdir,$(BINDIR),binaries)

$(TARGET): libs $(OBJ) | $(BINDIR)
	@echo generating $@.
	$(CC) $(LDFLAGS) $(OBJ) $(LIBS) -o $@

##################################### pre-processed (.i) and assembler (.s) output
.PHONY: cleanasmcpp

cleanasmcpp:
	@$(call rmfiles,$(ASMFILES) $(CPPFILES),asm and pre-processed)

%.i: %.c
	@echo generating $@ (cpp processed).
	@$(CC) -E $(CPPFLAGS) $(CFLAGS) $< -o $@

%.s: %.c
	@echo generating $@ (asm).
	@$(CC) -S -fverbose-asm $(CPPFLAGS) $(CFLAGS) $< -o $@

##################################### LSP (ccls)
.PHONY: ccls

ccls: $(CCLSFILE)

$(CCLSROOT):
	@echo creating project root file.
	@$(TOUCH) $@

# generate compile_commands.json.
# TODO: add Makefile dependencies.
# also, if cclsfile is newer than sources, no need to clean objects file
# (and to run bear).
# maybe run cleanobj cleanlibobj in commands ?
$(CCLSFILE): cleanobj cleanbrlib libs | $(CCLSROOT)
	@echo "Generating ccls compile commands file ($@)."
	@$(BEAR) -- $(MAKE) testing

##################################### valgrind (mem check)
.PHONY: memcheck

VALGRIND       = valgrind
VALGRINDFLAGS  = --leak-check=full --show-leak-kinds=all
VALGRINDFLAGS += --track-origins=yes --sigill-diagnostics=yes
VALGRINDFLAGS += --quiet --show-error-list=yes
VALGRINDFLAGS += --log-file=valgrind.out
# We need to suppress libreadline leaks here. See :
# https://stackoverflow.com/questions/72840015
VALGRINDFLAGS += --suppressions=etc/libreadline.supp

memcheck: targets
	@$(VALGRIND) $(VALGRINDFLAGS) $(BINDIR)/brchess

##################################### test binaries
.PHONY: testing test

TEST          := piece-test fen-test bitboard-test movegen-test attack-test movedo-test

PIECE_OBJS    := piece.o
FEN_OBJS      := fen.o position.o piece.o bitboard.o board.o hyperbola-quintessence.o \
	attack.o
BB_OBJS       := fen.o position.o piece.o bitboard.o board.o hyperbola-quintessence.o \
	attack.o
MOVEGEN_OBJS  := fen.o position.o piece.o bitboard.o board.o hyperbola-quintessence.o \
	attack.o move.o move-gen.o
ATTACK_OBJS   := fen.o position.o piece.o bitboard.o board.o hyperbola-quintessence.o \
	 attack.o move.o move-gen.o
MOVEDO_OBJS   := fen.o position.o piece.o bitboard.o board.o hyperbola-quintessence.o \
	 attack.o move.o move-gen.o move-do.o

TEST          := $(addprefix $(BINDIR)/,$(TEST))

PIECE_OBJS    := $(addprefix $(OBJDIR)/,$(PIECE_OBJS))
FEN_OBJS      := $(addprefix $(OBJDIR)/,$(FEN_OBJS))
BB_OBJS       := $(addprefix $(OBJDIR)/,$(BB_OBJS))
MOVEGEN_OBJS  := $(addprefix $(OBJDIR)/,$(MOVEGEN_OBJS))
ATTACK_OBJS   := $(addprefix $(OBJDIR)/,$(ATTACK_OBJS))
MOVEDO_OBJS   := $(addprefix $(OBJDIR)/,$(MOVEDO_OBJS))

test:
	echo TEST=$(TEST)
	echo FEN_OBJS=$(FEN_OBJS)

testing: $(TEST)

bin/piece-test: test/piece-test.c $(FEN_OBJS)
	@echo compiling $@ test executable.
	@$(CC) $(ALL_CFLAGS) $< $(FEN_OBJS) $(ALL_LDFLAGS) -o $@

bin/fen-test: test/fen-test.c test/common-test.h $(FEN_OBJS)
	@echo compiling $@ test executable.
	@$(CC) $(ALL_CFLAGS) $< $(FEN_OBJS) $(ALL_LDFLAGS) -o $@

bin/bitboard-test: test/bitboard-test.c test/common-test.h $(BB_OBJS)
	@echo compiling $@ test executable.
	@$(CC) $(ALL_CFLAGS) $< $(BB_OBJS) $(ALL_LDFLAGS) -o $@

bin/movegen-test: test/movegen-test.c test/common-test.h $(MOVEGEN_OBJS)
	@echo compiling $@ test executable.
	@$(CC) $(ALL_CFLAGS) $< $(MOVEGEN_OBJS) $(ALL_LDFLAGS) -o $@

bin/attack-test: test/attack-test.c test/common-test.h $(ATTACK_OBJS)
	@echo compiling $@ test executable.
	@$(CC) $(ALL_CFLAGS) $< $(ATTACK_OBJS) $(ALL_LDFLAGS) -o $@

bin/movedo-test: test/movedo-test.c test/common-test.h $(MOVEDO_OBJS)
	@echo compiling $@ test executable.
	@$(CC) $(ALL_CFLAGS) $< $(MOVEDO_OBJS) $(ALL_LDFLAGS) -o $@

##################################### Makefile debug
.PHONY: showflags wft

showflags:
	@echo CFLAGS: "$(CFLAGS)"
	@echo CPPFLAGS: $(CPPFLAGS)
	@echo DEPFLAGS: $(DEPFLAGS)
	@echo LDFLAGS: $(LDFLAGS)
	@echo DEPFLAGS: $(DEPFLAGS)

wtf:
	@printf "BRLIBDIR=%s\n" "$(BRLIBDIR)"
	@printf "LDFLAGS=%s\n\n" "$(LDFLAGS)"
	@#printf "LIBOBJ=%s\n\n" "$(LIBOBJ)"
	@#printf "OBJDIR=%s\n\n" "$(OBJDIR)"
	@#printf "OBJ=%s\n\n" "$(OBJ)"
	@#echo LIBOBJ=$(LIBOBJ)
	@#echo DEP=$(DEP)
	@#echo LIBSRC=$(LIBSRC)

zob:
	$(CC) $(LDFLAGS) $(CPPFLAGS) $(CFLAGS) $< $(LIBS) src/util.c -o util
