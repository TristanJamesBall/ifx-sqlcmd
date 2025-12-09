#	$Id: mkproc.mk,v 1.2 2003/11/07 23:58:20 jleffler Exp $
#
#	Makefile for MKPROC command

CC        = gcc -static
XFLAGS    = -Wall -Wshadow -Wstrict-prototypes -Wmissing-prototypes

#XFLAGS    =

ESQL       = INFORMIXC="${CC}" ${ESQL_CMD}
ESQL_CMD   = esql
ECFLAGS    =

RM        = rm -f
OFLAGS    = -O
PFLAGS    = -D_POSIX_SOURCE -D_POSIX_C_SOURCE=199309
INC1      = ${INFORMIXDIR}/incl/esql
INC2      = ${INFORMIXDIR}/incl
INCDIRS   = -I${INC1} -I${INC2}
STRIP     = -s
LDFLAGS   = ${STRIP}
CFLAGS    = ${SFLAGS} ${INCDIRS} ${OFLAGS} ${XFLAGS} ${PFLAGS}

#O         = obj
O         = o

# Cancel built-in suffix order - caused problems on Win2K?
.SUFFIXES:
.SUFFIXES: .ec .c .$O

.ec.$O:
	${ESQL} -c ${ECFLAGS} ${CFLAGS} $*.ec
	${RM} $*.c

PROGRAM = mkproc
FILES.o = mkproc.$O stderr.$O jlss_getopt.$O

all:	${PROGRAM}

${PROGRAM}:	${FILES.o}
	${ESQL} -o $@ ${FILES.o} ${LDFLAGS}

clean:
	${RM} ${PROGRAM} ${FILES.o}
