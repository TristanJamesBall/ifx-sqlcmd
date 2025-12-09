#   @(#)$Id: insblob.mk,v 2005.1 2005/02/24 03:02:44 jleffler Exp $
#
#   Makefile for INSBLOB program
#   Requires: insblob.ec
#       connblob.ec, connblob.h, fcopy.c, getline.c, getopt.c, getopt.h,
#       jlss.h, nstrcpy.c, sqlconn.ec, sqlconn.h, stderr.c, stderr.h,
#       strdotfill.c, strdotfill.h, tokencmp.c, tokencmp.h

ESQL      = esql
LDFLAGS   =
ESQLFLAGS = 
RM        = rm -f

INSBLOB.o = \
	insblob.o \
	connblob.o \
	emalloc.o \
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

all:	insblob

insblob:	${INSBLOB.o}
	${ESQL} -o $@ ${ESQLFLAGS} ${INSBLOB.o} ${LDFLAGS}

clean:
	${RM} insblob ${INSBLOB.o}

connblob.o: esqlinfo.h stderr.h connblob.h
