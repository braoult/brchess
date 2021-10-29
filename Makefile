#

BINDIR=./bin
SRCDIR=./src

SRC=$(wildcard $(SRCDIR)/*.c)
INC=$(wildcard $(SRCDIR)/*.h)
BIN=fen

CFLAGS += -std=c99
CFLAGS += -g
CFLAGS += -Wall
CFLAGS += -Wextra
CFLAGS += -pedantic
CFLAGS += -Wno-pointer-arith
#CFLAGS += -Werror
CFLAGS += -Wmissing-declarations

all: clean $(BIN)

.PHONY: clean
clean:
	rm -rf *.o core $(BIN)

fen: CFLAGS+=-DFENBIN
fen: $(SRC)
	echo SRC=$(SRC)
	$(CC) $(CFLAGS) $? -o $@
