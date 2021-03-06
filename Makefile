# ========================================================================================
# Makefile
# ========================================================================================

# ========================================================================================
# Compile flags

CC = gcc
COPT = -O2
CFLAGS = -Wall -Wextra -Wpedantic -Werror -std=gnu11 -D_GNU_SOURCE
CFLAGS += -D BUILD_VERSION="\"$(shell git describe --dirty --always)\""	\
		-D BUILD_DATE="\"$(shell date '+%Y-%m-%d_%H:%M:%S')\""

BIN = touchdemo

# ========================================================================================
# Source files

SRCDIR = .

SRC = $(SRCDIR)/touchdemo.c

# ========================================================================================
# External Libraries

LIBSDIR = 
LIBS = 

# ========================================================================================
# Makerules

all:
	$(CC) $(COPT) $(CFLAGS) $(SRC) -o $(BIN) $(LIBSDIR) $(LIBS)

debug: COPT = -Og -gdwarf -fno-omit-frame-pointer -D__DEBUG
debug: all

clean:
	rm -fv *.o $(BIN)
