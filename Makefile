LIBSRC=$(wildcard codecs/*.c containers/*.c *.c)
LIBOBJ=$(LIBSRC:.c=.o)
LIBOUT=libtex.a

BINSRC=texmex.c
BINOBJ=$(BINSRC:.c=.o)
BINOUT=texmex

# C compiler flags (-g -O2 -Wall)
CFLAGS=-g -O0 -Wall

# compiler
CC=gcc

# compile flags
LDFLAGS=-g

default: $(BINOUT)

$(BINOUT): $(LIBOUT) $(BINOBJ)
	$(CC) -o $@ $(BINOBJ) -L. -ltex -lc -lstdc++ -lm

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

$(LIBOUT): $(LIBOBJ)
	ar rcs $(LIBOUT) $(LIBOBJ)

clean: cleanlib cleanbin
	rm -f Makefile.bak

cleanlib:
	rm -f $(LIBOBJ) $(LIBOUT)

cleanbin:
	rm -f $(BINOBJ) $(BINOUT)

