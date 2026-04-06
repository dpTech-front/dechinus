# Dechinus WM version
VERSION = 0.1.0

# Real User's home detection
ifeq ($(SUDO_USER),)
	REAL_HOME = ${HOME}
else
	REAL_HOME = $(shell eval echo ~${SUDO_USER})
endif

# installation paths 
PREFIX?= /usr/local
BINPREFIX?= ${PREFIX}/bin
MANPREFIX?= ${PREFIX}/share/man
DOCPREFIX?= ${PREFIX}/share/doc
CONFPREFIX?= ${REAL_HOME}/.config/dewm
CONF?= ${CONFPREFIX}

# X11 Paths
X11INC?= /usr/X11R6/include
X11LIB?= /usr/X11R6/lib

# includes and libs
INCS = -I. -I/usr/include -I${X11INC} `pkg-config --cflags xft`
LIBS = -L/usr/lib -lc -L${X11LIB} -lX11 `pkg-config --libs xft`

DEFS = -DVERSION=\"${VERSION}\" -DSYSCONFPATH=\"${CONF}\"

# cimpilation flags
CFLAGS = -Os ${INCS} ${DEFS}
LDFLAGS = -s ${LIBS}

# debug flags
CFLAGS = -g3 -ggdb3 -std=c99 -pedantic -O0 ${INCS} -DDEBUG ${DEFS}
LDFLAGS = -g3 -ggdb3 ${LIBS}

# DEBUG: Show warnings (if any). Comment out to disable.
#CFLAGS += -Wall -Wpadded
# mostly useless warnings
#CFLAGS += -W -Wcast-qual -Wshadow -Wwrite-strings
#CFLAGS += -Werror        # Treat warnings as errors.
#CFLAGS += -save-temps    # Keep precompiler output (great for debugging).

# XRandr (multihead support). Comment out to disable.
CFLAGS += -DXRANDR=1
LIBS += -lXrandr

# compiler and linker
#CC = cc
