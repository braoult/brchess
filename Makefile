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

BIN=fen piece move eval brchess

LIBS       = -l$(LIB) -lreadline -lncurses

CFLAGS += -std=gnu11

#CFLAGS += -O2
CFLAGS    += -g
CFLAGS    += -Wall
CFLAGS    += -Wextra
CFLAGS    += -march=native
CFLAGS    += -Wmissing-declarations

##################################### DEBUG flags
CPPFLAGS   = -I$(INCDIR)
CPPFLAGS  += -DDEBUG            # global
CPPFLAGS  += -DDEBUG_DEBUG      # enable log() functions
CPPFLAGS  += -DDEBUG_POOL       # memory pools management
CPPFLAGS  += -DDEBUG_FEN                # FEN decoding
CPPFLAGS  += -DDEBUG_MOVE       # move generation
CPPFLAGS  += -DDEBUG_EVAL       # eval functions
CPPFLAGS  += -DDEBUG_PIECE      # piece list management

##################################### General targets
.PHONY: compile cflags all clean cleanall

compile: libs objects bin

cflags:
	@echo CFLAGS: "$(CFLAGS)"
	@echo CPPFLAGS: $(CPPFLAGS)
	@echo DEPFLAGS: $(DEPFLAGS)
	@echo LDFLAGS: $(LDFLAGS)

all: clean compile

clean: cleanobj cleanbin

cleanall: clean cleandeps cleanlib

##################################### Dependencies
.PHONY: deps cleandeps

DEPDIR := ./.deps
DEPFILES := $(addprefix $(DEPDIR)/,$(SRC_S:.c=.d))
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
OBJ=$(addprefix $(OBJDIR)/,$(SRC_S:.c=.o))

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

##################################### binaries
.PHONY: bin cleanbin

bin: $(BIN)

cleanbin:
	$(RM) -f $(BIN)

# TODO: find a better dependancy graph
$(BIN): $$(subst $(OBJDIR)/$$@.o,,$(OBJ)) $(SRCDIR)/$$@.c
	@echo generating $@.
	@#echo NEED_TO_CHANGE_THIS=$^
	@$(CC) -DBIN_$@ $(CFLAGS) -I $(INCDIR) $^ $(LDFLAGS) $(LIBS) -o $@

##################################### br library
.PHONY: cleanlib libs

ARFLAGS = r

cleanlib:
	$(RM) -rf $(LIBDIR) $(LIBOBJ)

$(LIBDIR):
	@echo creating $@ directory.
	@mkdir -p $@

libs: $(DLIB) $(SLIB) | $(LIBDIR)

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
