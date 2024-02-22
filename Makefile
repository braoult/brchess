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
LD        := ld
BEAR      := bear
TOUCH     := touch
RM        := rm
RMDIR     := rmdir
MAKE      := make

SRCDIR    := ./src
INCDIR    := ./src
OBJDIR    := ./obj
BINDIR    := ./bin
DEPDIR    := ./dep

BRLIB     := ./brlib
BRINCDIR  := $(BRLIB)/include
BRLIBDIR  := $(BRLIB)/lib

CCLSROOT  := .ccls-root
CCLSFILE  := compile_commands.json

SRC       := $(wildcard $(SRCDIR)/*.c)                      # project sources
SRC_FN    := $(notdir $(SRC))                               # source basename
OBJ       := $(addprefix $(OBJDIR)/,$(SRC_FN:.c=.o))

LIB       := br_$(shell uname -m)			     # library name

DEP_FN    := $(SRC_FN) $(LIBSRC_FN)
DEP       := $(addprefix $(DEPDIR)/,$(DEP_FN:.c=.d))

TARGET_FN := brchess
TARGET    := $(addprefix $(BINDIR)/,$(TARGET_FN))

LDFLAGS   := -L$(BRLIBDIR)
LIBS      := $(strip -l$(LIB) -lreadline)

##################################### pre-processor flags
CPPFLAGS  := -I$(BRINCDIR) -I$(INCDIR)
CPPFLAGS  += -DBUG_ON
CPPFLAGS  += -DWARN_ON
CPPFLAGS  += -DNDEBUG

#CPPFLAGS  += -DDEBUG                         # global
CPPFLAGS  += -DDEBUG_DEBUG                   # enable log() functions
#CPPFLAGS  += -DDEBUG_DEBUG_C                # enable verbose log() settings
CPPFLAGS  += -DDEBUG_POOL                    # memory pools management
#CPPFLAGS  += -DDEBUG_FEN                     # FEN decoding
CPPFLAGS  += -DDEBUG_MOVE                    # move generation
CPPFLAGS  += -DDEBUG_EVAL                    # eval functions
CPPFLAGS  += -DDEBUG_PIECE                   # piece list management
CPPFLAGS  += -DDEBUG_SEARCH                  # move search

# remove extraneous spaces (due to spaces before comments)
CPPFLAGS  := $(strip $(CPPFLAGS))

##################################### compiler flags
CFLAGS    := -std=gnu11
CFLAGS    += -O3
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

##################################### archiver/dependency flags
ARFLAGS   := rcs
DEPFLAGS  = -MMD -MP -MF $(DEPDIR)/$*.d

##################################### General targets
.PHONY: all compile clean cleanall

all: $(TARGET)

compile: brlib objs

libs: brlib

clean: cleandep cleanobj cleanbin

cleanall: clean cleandepdir cleanobjdir cleanallbrlib cleanbindir

##################################### cleaning functions
# rmfiles - deletes a list of files in a directory if they exist.
# $(1): the directory
# $(2): the list of files to delete
# $(3): The string to include in action output - "cleaning X files."
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

# rmdir - deletes a directory if it exists.
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
	@$(CC) -c $(DEPFLAGS) $(CPPFLAGS) $(CFLAGS) $< -o $@

##################################### brlib libraries
.PHONY: cleanbrlib cleanallbrlib libs brlib

cleanbrlib:
	$(MAKE) -C $(BRLIB) clean
	@#$(call rmfiles,$(DLIB) $(SLIB),library)

cleanallbrlib:
	$(MAKE) -C $(BRLIB) cleanall

brlib: libs
	$(MAKE) -C $(BRLIB) libs

##################################### brchess binaries
.PHONY: targets cleanbin cleanbindir

targets: $(TARGET)

cleanbin:
	$(call rmfiles,$(TARGET),binary)

cleanbindir:
	$(call rmdir,$(BINDIR),binaries)

# We don't use static lib, but we could build it here
#$(TARGET): $(DLIB) $(OBJ) | $(BINDIR) $(SLIB)
$(TARGET): libs $(OBJ) | $(BINDIR) $(SLIB)
	@echo generating $@ executable.
	$(CC) $(LDFLAGS) $(OBJ) $(LIBS) -o $@

##################################### pre-processed (.i) and assembler (.s) output
%.i: %.c
	@echo generating $@
	@$(CC) -E $(CPPFLAGS) $(CFLAGS) $< -o $@

%.s: %.c
	@echo generating $@
	@$(CC) -S -fverbose-asm $(CPPFLAGS) $(CFLAGS) $< -o $@

##################################### LSP (ccls)
.PHONY: ccls

ccls: $(CCLSFILE)

$(CCLSROOT):
	@echo creating project root file.
	@$(TOUCH) $@

# generate compile_commands.json.
# Need to add includes and Makefile dependencies.
# also, if cclsfile is newer than sources, no need to clean objects file
# (and to run bear).
# maybe run cleanobj cleanlibobj in commands ?
$(CCLSFILE): cleanobj cleanbrlib $(SRC) $(LIBSRC) | $(CCLSROOT)
	@echo "Generating ccls compile commands file ($@)."
	@$(BEAR) -- $(MAKE) compile

#.PHONY: bear
#bear: cleanobj cleanlibobj Makefile | $(CCLSROOT)
#    @$(BEAR) -- make compile

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
TEST        = bin/fen-test bin/bitboard-test

FENTESTOBJS = obj/fen.o obj/position.o obj/piece.o obj/util.o obj/bitboard.o
BITBOARDOBJS = obj/position.o obj/piece.o obj/bitboard.o obj/fen.o \
	obj/hyperbola-quintessence.o

testing: $(TEST)

bin/fen-test: test/fen-test.c $(FENTESTOBJS)
	$(CC) $(CPPFLAGS) $(CFLAGS) $< $(FENTESTOBJS)  $(LDFLAGS) $(LIBS) -o $@

bin/bitboard-test: test/bitboard-test.c $(BITBOARDOBJS)
	$(CC) $(CPPFLAGS) $(CFLAGS) $< $(BITBOARDOBJS) $(LDFLAGS) $(LIBS) -o $@


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
