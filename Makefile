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

BEAR      := bear
TOUCH     := touch
RM        := rm
RMDIR     := rmdir
MAKE      := make

SRCDIR    := ./src
INCDIR    := ./src                                          # used by ./test sources
OBJDIR    := ./obj
BINDIR    := ./bin
DEPDIR    := ./dep
TSTDIR    := ./test

BRLIB     := ./brlib
BRINCDIR  := $(BRLIB)/include
BRLIBDIR  := $(BRLIB)/lib

CCLSROOT  := .ccls-root
CCLSFILE  := compile_commands.json

SRC       := $(wildcard $(SRCDIR)/*.c)                      # project sources
SRC_FN    := $(notdir $(SRC))                               # source basename
OBJ       := $(addprefix $(OBJDIR)/,$(SRC_FN:.c=.o))

TSTSRC    := $(wildcard $(TSTDIR)/*.c)

LIB       := br_$(shell uname -m)                           # library name
LIBS      := $(strip -l$(LIB))

DEP_FN    := $(SRC_FN)
DEP       := $(addprefix $(DEPDIR)/,$(DEP_FN:.c=.d))

TARGET_FN := brchess
TARGET    := $(addprefix $(BINDIR)/,$(TARGET_FN))

ASMFILES  := $(SRC:.c=.s) $(TSTSRC:.c=.s)
CPPFILES  := $(SRC:.c=.i) $(TSTSRC:.c=.i)

##################################### Check for compiler and requested build
BUILDS    := release dev perf debug
# last compilation build
BUILDFILE := .lastbuild
lastbuild := $(file < $(BUILDFILE))

# default to gcc
CC        ?= cc
ifeq ($(CC),cc)
        CC = gcc
endif

# if no build specified, use last one
ifeq ($(build),)
        build := $(lastbuild)
endif
# if build is still undefined, set a default
ifeq ($(build),)
        build := release
endif

# check for valid build
ifeq ($(filter $(build),$(BUILDS)),)
        $(error Error: Unknown build=`$(build)`. Possible builds are: $(BUILDS))
endif

# if new build, rewrite BUILDFILE
ifeq ($(build),$(lastbuild))
        $(info Using last used build:`$(build)`.)
else
        $(info Using new build:`$(build)` (previous:$(lastbuild)))
        $(file >$(BUILDFILE),$(build))
endif

##################################### set a version string
# inspired from:
#  https://eugene-babichenko.github.io/blog/2019/09/28/nightly-versions-makefiles/

# last commit and date
COMMIT    := $(shell git rev-parse --short HEAD)
DATE      := $(shell git log -1 --format=%cd --date=format:"%Y%m%d")

# get last commit w/ tag & associated tag, if any
TAG_COMM  := $(shell git rev-list --abbrev-commit --tags --max-count=1)
ifneq ($(TAG_COMMIT),)
TAG       := $(shell git describe --abbrev=0 --tags ${TG_COMM} 2>/dev/null || true)
VERSION   := $(TAG:v%=%)
endif

# if no version, use last commit and date.
# else, if last commit != last tag commit, add commit and date to version number
ifeq ($(VERSION),)
  VERSION := $(build)-$(COMMIT)-$(DATE)
else ifneq ($(COMMIT), $(TAG_COMMIT))
  VERSION := $(VERSION)-next-$(build)-$(COMMIT)-$(DATE)
endif
# if uncommited changes, add "dirty" indicator
ifneq ($(shell git status --porcelain),)
  VERSION := $(VERSION)-dirty
endif

##################################### pre-processor flags
CPPFLAGS  := -I$(BRINCDIR) -I$(INCDIR) -DVERSION=\"$(VERSION)\"

CPPFLAGS  += -DDIAGRAM_SYM                                  # UTF8 symbols in diagrams

ifeq ($(build),release)
        CPPFLAGS  += -DNDEBUG                               # assert (unused)
else # ifeq ($(build),dev)
        CPPFLAGS  += -DBUG_ON                               # brlib bug.h

        #        fen.c
        #CPPFLAGS  += -DDEBUG_FEN                           # FEN decoding
        #        hash / TT
        #CPPFLAGS  += -DZOBRIST_VERIFY                      # double chk zobrist
        #CPPFLAGS  += -DPERFT_MOVE_HISTORY                  # perft, keep prev moves
        #        attack.c
        #CPPFLAGS  += -DDEBUG_ATTACK_ATTACKERS              # sq_attackers
        #CPPFLAGS  += -DDEBUG_ATTACK_PINNERS                # sq_pinners details
        #        eval.c eval-simple.c
        CPPFLAGS  += -DEVAL                                 # eval

        #        old unused flags
        #CPPFLAGS  += -DDEBUG_POS                           # position.c
        #CPPFLAGS  += -DDEBUG_MOVE                          # move generation
        #CPPFLAGS  += -DDEBUG_EVAL                          # eval functions
endif

# remove extraneous spaces (due to spaces before comments)
CPPFLAGS  := $(strip $(CPPFLAGS))

##################################### compiler / linker flags
CFLAGS    := -std=gnu11

CFLAGS    += -Wall -Wextra -Wshadow -Wmissing-declarations
CFLAGS    += -march=native

LDFLAGS   := --static
LDFLAGS   += -L$(BRLIBDIR)

### dev OR release
ifeq ($(build),release)
        CFLAGS    += -O3
        CFLAGS    += -funroll-loops
        CFLAGS    += -flto
        #CFLAGS    += -g
        #CFLAGS    += -ginline-points                   # inlined funcs debug info
        LDFLAGS   += -flto
else ifeq ($(build),dev)
        CFLAGS    += -Og
        CFLAGS    += -g                                # symbols (gdb, perf, etc.)
        CFLAGS    += -ginline-points                   # inlined funcs debug info
        #CFLAGS += -pg                                 # gprof
        # Next one may be useful for valgrind (when invalid instructions)
        #CFLAGS += -mno-tbm
else ifeq ($(build),perf)
        CFLAGS    += -O3
        CFLAGS    += -g                                # symbols (gdb, perf, etc.)
        CFLAGS    += -ginline-points                   # inlined funcs debug info
        CFLAGS    += -funroll-loops
else ifeq ($(build),debug)
        CFLAGS    += -Og
        CFLAGS    += -g                                # symbols (gdb, perf, etc.)
        CFLAGS    += -ginline-points                   # inlined funcs debug info
        # for gprof
        #CFLAGS += -pg
        # Next one may be useful for valgrind (when invalid instructions)
        #CFLAGS += -mno-tbm
endif

CFLAGS    := $(strip $(CFLAGS))
LDFLAGS   := $(strip $(LDFLAGS))

##################################### dependency flags
DEPFLAGS   = -MMD -MP -MF $(DEPDIR)/$*.d

##################################### archiver/dependency flags
ALL_CFLAGS  = $(DEPFLAGS) $(CPPFLAGS) $(CFLAGS)
ALL_LDFLAGS = $(LDFLAGS) $(LIBS)

##################################### Multi-targets
# We can have an issue with Make's "-j" option, in some situations,
# for example "make -j2 clean testing".
# See https://stackoverflow.com/a/8496333/3079831

# Check if job server supported:
ifeq ($(filter jobserver, $(.FEATURES)),)
# Job server not supported: sub-makes will only start one job unless
# you specify a higher number here.  Here we use a MS Windows environment
# variable specifying number of processors.
JOBSARG := -j $(NUMBER_OF_PROCESSORS)
else
# Job server is supported; let GNU Make work as normal.
JOBSARG :=
endif

# .FEATURES only works in GNU Make 3.81+.
# If GNU make is older, assume job server support.
ifneq ($(firstword $(sort 3.81 $(MAKE_VERSION))),3.81)
# If you are using GNU Make < 3.81 that does not support job servers, you
# might want to specify -jN parameter here instead.
JOBSARG :=
endif

ifneq ($(words $(MAKECMDGOALS)),1)
.NOTPARALLEL:
# The "all" target is required in the list,
# in case user invokes make with no targets.
$(sort all $(MAKECMDGOALS)):
	@$(MAKE) $(JOBSARG) -f $(firstword $(MAKEFILE_LIST)) $@
else

##################################### General targets
.PHONY: all release dev perf debug compile libs clean cleanall

all: libs testing $(TARGET)

release:
	$(MAKE) BUILD=release clean all

dev:
	$(MAKE) BUILD=dev clean all

perf:
	$(MAKE) BUILD=perf clean all

debug:
	$(MAKE) BUILD=debug clean all

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

# Don't use "rm $(DEPDIR)/*.d", to understand mismatches between dep/ and src/
# files.
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
$(OBJDIR)/%.o: $(BUILDFILE)
$(OBJDIR)/%.o: $(SRCDIR)/%.c $(BUILDFILE) | $(OBJDIR) $(DEPDIR)
	@echo compiling brchess module: $< "->" $@.
	$(CC) -c $(ALL_CFLAGS) $< -o $@

##################################### brlib libraries
.PHONY: cleanbrlib cleanallbrlib brlib

cleanbrlib:
	$(MAKE) -C $(BRLIB) clean

cleanallbrlib:
	$(MAKE) -C $(BRLIB) cleanall

export build
brlib:
	$(info calling with build=$(build))
	$(MAKE) -e -C $(BRLIB) lib-static
unexport build
##################################### brchess binaries
.PHONY: targets cleanbin cleanbindir

targets: $(TARGET)

cleanbin:
	$(call rmfiles,$(TARGET),binary)

cleanbindir:
	$(call rmdir,$(BINDIR),binaries)

$(TARGET): libs $(OBJ) | $(BINDIR)
	@echo linking $@.
	$(CC) $(LDFLAGS) $(OBJ) $(LIBS) -o $@

##################################### pre-processed (.i) and assembler (.s) output
.PHONY: cleanasmcpp

cleanasmcpp:
	@$(call rmfiles,$(ASMFILES) $(CPPFILES),asm and pre-processed)

%.i: %.c
	@echo "generating $@ (cpp processed)."
	@$(CC) -E $(CPPFLAGS) $(CFLAGS) $< -o $@

%.s: %.c
	@echo "generating $@ (asm)."
	$(CC) -S -fverbose-asm $(CPPFLAGS) $(CFLAGS) $< -o $@

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
	@$(BEAR) -- $(MAKE)

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
.PHONY: testing

TEST          := piece-test fen-test bitboard-test movegen-test attack-test
TEST          += movedo-test perft-test tt-test

PIECE_OBJS    := piece.o
FEN_OBJS      := $(PIECE_OBJS) fen.o position.o bitboard.o board.o \
	hq.o attack.o hash.o init.o misc.o alloc.o move.o eval-simple.o
BB_OBJS       := $(FEN_OBJS)
MOVEGEN_OBJS  := $(BB_OBJS) move-gen.o
ATTACK_OBJS   := $(MOVEGEN_OBJS)
MOVEDO_OBJS   := $(ATTACK_OBJS) move-do.o
PERFT_OBJS    := $(MOVEDO_OBJS) search.o
TT_OBJS       := $(MOVEDO_OBJS)

TEST          := $(addprefix $(BINDIR)/,$(TEST))

PIECE_OBJS    := $(addprefix $(OBJDIR)/,$(PIECE_OBJS))
FEN_OBJS      := $(addprefix $(OBJDIR)/,$(FEN_OBJS))
BB_OBJS       := $(addprefix $(OBJDIR)/,$(BB_OBJS))
MOVEGEN_OBJS  := $(addprefix $(OBJDIR)/,$(MOVEGEN_OBJS))
ATTACK_OBJS   := $(addprefix $(OBJDIR)/,$(ATTACK_OBJS))
MOVEDO_OBJS   := $(addprefix $(OBJDIR)/,$(MOVEDO_OBJS))
PERFT_OBJS    := $(addprefix $(OBJDIR)/,$(PERFT_OBJS))
TT_OBJS       := $(addprefix $(OBJDIR)/,$(TT_OBJS))

testing: $(TEST)

bin/piece-test: test/piece-test.c $(FEN_OBJS)
	@echo linking $@ test executable.
	@$(CC) $(ALL_CFLAGS) $< $(FEN_OBJS) $(ALL_LDFLAGS) -o $@

bin/fen-test: test/fen-test.c test/common-test.h $(FEN_OBJS)
	@echo linking $@ test executable.
	@$(CC) $(ALL_CFLAGS) $< $(FEN_OBJS) $(ALL_LDFLAGS) -o $@

bin/bitboard-test: test/bitboard-test.c test/common-test.h $(BB_OBJS)
	@echo linking $@ test executable.
	@$(CC) $(ALL_CFLAGS) $< $(BB_OBJS) $(ALL_LDFLAGS) -o $@

bin/movegen-test: test/movegen-test.c test/common-test.h $(MOVEGEN_OBJS)
	@echo linking $@ test executable.
	@$(CC) $(ALL_CFLAGS) $< $(MOVEGEN_OBJS) $(ALL_LDFLAGS) -o $@

bin/attack-test: test/attack-test.c test/common-test.h $(ATTACK_OBJS)
	@echo linking $@ test executable.
	@$(CC) $(ALL_CFLAGS) $< $(ATTACK_OBJS) $(ALL_LDFLAGS) -o $@

bin/movedo-test: test/movedo-test.c test/common-test.h $(MOVEDO_OBJS)
	@echo linking $@ test executable.
	@$(CC) $(ALL_CFLAGS) $< $(MOVEDO_OBJS) $(ALL_LDFLAGS) -o $@

bin/perft-test: test/perft-test.c test/common-test.h $(PERFT_OBJS)
	@echo linking $@ test executable.
	@$(CC) $(ALL_CFLAGS) $< $(PERFT_OBJS) $(ALL_LDFLAGS) -o $@

bin/tt-test: test/tt-test.c test/common-test.h $(TT_OBJS)
	@echo linking $@ test executable.
	@$(CC) $(ALL_CFLAGS) $< $(TT_OBJS) $(ALL_LDFLAGS) -o $@

##################################### Makefile debug
.PHONY: showflags wft

info:
	@printf "CFLAGS:   +%s+\n" "$(CFLAGS)"
	@printf "CPPFLAGS: +%s+\n" "$(CPPFLAGS)"
	@printf "DEPFLAGS: +%s+\n" "$(DEPFLAGS)"
	@printf "LDFLAGS:  +%s+\n" "$(LDFLAGS)"
	@printf "DEPFLAGS: +%s+\n" "$(DEPFLAGS)"
	@printf "VERSION:  +%s+\n" "$(VERSION)"

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
	@#$(CC) $(LDFLAGS) $(CPPFLAGS) $(CFLAGS) $< $(LIBS) src/util.c -o util

##################################### End of multi-targets
endif
