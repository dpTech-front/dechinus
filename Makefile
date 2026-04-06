# Dechinus WM - Sharp, minimal, and fast - a tiling window for X11
# Original Echinus WM 
# © 2006-2007 Anselm R. Garbe, Sander van Dijk
# © 2008 Alexander Polakov
#
# Dechinus WM fork and maintenance:
# C 2026 Daniel B. Prodigalidad

include config.mk

PIXMAPS = close.xbm iconify.xbm max.xbm
SRC = draw.c dechinus.c ewmh.c parse.c
HEADERS = config.h dechinus.h
OBJ = ${SRC:.c=.o}

all: options dewm ${HEADERS}

options:
	@echo Dechinus build options:
	@echo "CFLAGS   = ${CFLAGS}"
	@echo "LDFLAGS  = ${LDFLAGS}"
	@echo "CC       = ${CC}"

.c.o:
	@echo CC $<
	@${CC} -c ${CFLAGS} $<

${OBJ}: config.mk ${HEADERS}

dewm: ${OBJ} ${SRC} ${HEADERS}
	@echo CC -o $@
	@${CC} -o $@ ${OBJ} ${LDFLAGS}

clean:
	@echo cleaning
	@rm -f dewm ${OBJ} dewm-${VERSION}.tar.gz *~

dist: clean
	@echo creating dist tarball
	@mkdir -p dewm-${VERSION}
	@cp -R LICENSE Makefile README.md config.mk \
		echinus.1 dewmrc ${SRC} ${HEADERS} ${PIXMAPS} dewm-${VERSION}
	@tar -cf dewm-${VERSION}.tar dewm-${VERSION}
	@gzip dewm-${VERSION}.tar
	@rm -rf dewm-${VERSION}

install: all
	@echo installing executable file to ${DESTDIR}${BINPREFIX}
	@mkdir -p ${DESTDIR}${BINPREFIX}
	@cp -f dewm ${DESTDIR}${BINPREFIX}
	@chmod 755 ${DESTDIR}${BINPREFIX}/dewm
	@echo installing configuration file and pixmaps to ${DESTDIR}${CONFPREFIX}
	@mkdir -p ${DESTDIR}${CONFPREFIX}
	@cp dewmrc ${DESTDIR}${CONFPREFIX}/dewmrc
	@cp ${PIXMAPS} ${DESTDIR}${CONFPREFIX}
	@echo installing manual page to ${DESTDIR}${MANPREFIX}/man1
	@mkdir -p ${DESTDIR}${MANPREFIX}/man1
	@sed "s/VERSION/${VERSION}/g;s|CONFDIR|${DESTDIR}${CONF}|g" < dechinus.1 > ${DESTDIR}${MANPREFIX}/man1/dewm.1
	@echo installing README to ${DESTDIR}${DOCPREFIX}/dewm
	@mkdir -p ${DESTDIR}${DOCPREFIX}/dewm
	@sed "s|CONFDIR|${CONF}|" < README.md > ${DESTDIR}${DOCPREFIX}/dewm/README.md

uninstall:
	@echo removing executable file from ${DESTDIR}${BINPREFIX}
	@rm -f ${DESTDIR}${BINPREFIX}/dewm
	@echo removing manual page from ${DESTDIR}${MANPREFIX}/man1
	@rm -f ${DESTDIR}${MANPREFIX}/man1/dewm.1
	@echo removing configuration file and pixmaps from ${DESTDIR}${CONFPREFIX}
	@rm -rf ${DESTDIR}${CONFPREFIX}
	@echo removing documentation from  ${DESRDIR}${DOCPREFIX}/dewm
	@rm -rf ${DESRDIR}${DOCPREFIX}/dewm

.PHONY: all options clean dist install uninstall
