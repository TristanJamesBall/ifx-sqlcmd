#	@(#)$Id: updblob.mk,v 2005.1 2005/02/24 03:02:44 jleffler Exp $
#
#	Makefile for UPDBLOB program
#	Requires: updblob.ec
#       connblob.ec, connblob.h, fcopy.c, getline.c, getopt.c, getopt.h,
#       jlss.h, nstrcpy.c, sqlconn.ec, sqlconn.h, stderr.c, stderr.h,
#       strdotfill.c, strdotfill.h, tokencmp.c, tokencmp.h

ESQL      = esql
ESQLFLAGS = 
LDFLAGS   =
RM        = rm -f

UPDBLOB.o = \
	updblob.o \
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

all:	updblob

updblob:	${UPDBLOB.o}
	${ESQL} -o $@ ${ESQLFLAGS} ${UPDBLOB.o} ${LDFLAGS}

clean:
	${RM} updblob ${UPDBLOB.o}

connblob.o: esqlinfo.h stderr.h connblob.h
