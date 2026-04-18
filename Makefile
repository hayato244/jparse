CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -Wswitch -g

BINDIR = bin
OBJDIR = obj
SRCDIR = src

_OBJ = main.o reader.o arena.o helper.o lexer.o
OBJ = $(patsubst %,$(OBJDIR)/%,$(_OBJ))

_BIN = main
BIN = $(patsubst %,$(BINDIR)/%,$(_BIN))

all: $(BIN)

obj:
	mkdir $@

bin:
	mkdir $@

$(OBJDIR)/%.o: $(SRCDIR)/%.c | obj
	$(CC) $(CFLAGS) -c -o $@ $< 

$(BINDIR)/%: $(OBJ) | bin
	$(CC) $(CFLAGS) -o $@ $(OBJ) 

.PHONY: clean

clean:
	rm -f $(BIN) $(OBJDIR)/*.o core
