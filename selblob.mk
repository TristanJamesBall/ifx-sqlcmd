#   @(#)$Id: selblob.mk,v 2005.1 2005/02/24 03:02:44 jleffler Exp $
#
#   Makefile for SELBLOB program
#   Requires: selblob.ec
#       connblob.ec, connblob.h, fcopy.c, getline.c, getopt.c, getopt.h,
#       jlss.h, nstrcpy.c, sqlconn.ec, sqlconn.h, stderr.c, stderr.h,
#       strdotfill.c, strdotfill.h, tokencmp.c, tokencmp.h

ESQL      = esql
LDFLAGS   =
ESQLFLAGS = 
RM        = rm -f

SELBLOB.o = \
	selblob.o \
	connblob.o \
	getline.o \
	getopt.o \
	nstrcpy.o \
	sqlconn.o \
	stderr.o \
	strdotfill.o \
	tokencmp.o

.SUFFIXES:
.SUFFIXES: .ec .c .o

.ec.o:
	${ESQL} ${ESQLFLAGS} -c $*.ec
	${RM} $*.c

all:	selblob

selblob:	${SELBLOB.o}
	${ESQL} -o $@ ${ESQLFLAGS} ${SELBLOB.o} ${LDFLAGS}

clean:
	${RM} selblob ${SELBLOB.o}

connblob.o: esqlinfo.h stderr.h connblob.h
