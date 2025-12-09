#   @(#)$Id: selmultiblob.mk.in,v 2013.1 2013/12/28 22:50:05 jleffler Exp $
#
#   Makefile for SELMULTIBLOB program
#   Requires: selmultiblob.ec
#       basename.c, connblob.ec, connblob.h, debug.c, debug.h,
#       emalloc.c, emalloc.h, esnprintf.c, esnprintf.h, esqlutil.h,
#       esqlcver.ec, estrdup.c, fcopy.c, getline.c, getopt.c, getopt.h,
#       ilog10.c, ilog10.h, ixblob.ec, ixblob.h, jlss.h, kludge.c,
#       kludge.h, mkpath.c, nstrcpy.c, sqlconn.ec, sqlconn.h,
#       sqltype.ec, stderr.c, stderr.h, strdotfill.c, strdotfill.h,
#       tokencmp.c, tokencmp.h

include esqlc.mk

OFLAGS        = -O
CC            = gcc
ESQLC_COMMAND = esql
ESQLC_VERSION = 450
ESQLC_VERTEXT = IBM Informix CSDK Version 4.50, IBM Informix-ESQL Version 4.50.FC11
ESQLC_INCLDIR = /opt/ifx.sdk.450.fc11//incl/esql
ESQLC_AIXLOCT = 
INFORMIXDIR   = /opt/ifx.sdk.450.fc11/
HAVE_CONFIG_H = -DHAVE_CONFIG_H
RANLIB        = ranlib
OFILES.o      = 
LIBS	      = 
USER_CPPFLAGS = 
USER_CFLAGS   = -static -O3
USER_LDFLAGS  = -static

UFLAGS    = # User flags for C compiler; always overridable on command line
PFLAGS    = ${HAVE_CONFIG_H} ${ESQLC_AIXLOCT}
VERSION   = -DESQLC_VERSION=${ESQLC_VERSION}
INCDIRS   = -I. -I${ESQLC_INCLDIR}
STRIP     = #-s
AUXLIBS   =
LDFLAGS   = ${AUXLIBS} ${STRIP} ${USER_LDFLAGS}
CFLAGS    = ${UFLAGS} ${VERSION} ${INCDIRS} ${OFLAGS} ${XFLAGS} ${PFLAGS} \
${USER_CPPFLAGS} ${USER_CFLAGS}

ESQL_CMD  = ${ESQLC_COMMAND}
ESQLFLAGS = ${CFLAGS}
RM        = rm -f
MKDEP     = mkdep

FILES.src = \
	selmultiblob.ec \
	connblob.ec \
	debug.c \
	emalloc.c \
	esnprintf.c \
	esqlcver.ec \
	estrdup.c \
	getline.c \
	getopt.c \
	ilog10.c \
	ixblob.ec \
	kludge.c \
	mkpath.c \
	nstrcpy.c \
	sqlconn.ec \
	sqltype.ec \
	stderr.c \
	strdotfill.c \
	tokencmp.c

PROGRAM   = selmultiblob

FILES.o   = \
	selmultiblob.o \
	basename.o \
	connblob.o \
	debug.o \
	emalloc.o \
	esnprintf.o \
	esqlcver.o \
	estrdup.o \
	getline.o \
	getopt.o \
	ilog10.o \
	ixblob.o \
	kludge.o \
	mkpath.o \
	nstrcpy.o \
	sqlconn.o \
	sqltype.o \
	stderr.o \
	strdotfill.o \
	tokencmp.o

all:	${PROGRAM}

${PROGRAM}:	${FILES.o}
	${ESQL} -o $@ ${FILES.o} ${LDFLAGS}

clean:
	${RM} ${PROGRAM} ${FILES.o}

depend:
	${MKDEP} -fselmultiblob.mk ${FILES.src}

# DO NOT DELETE THIS LINE or the blank line after it -- make depend uses them.

basename.o: jlss.h
connblob.o: connblob.h
connblob.o: esqlinfo.h 
connblob.o: stderr.h
emalloc.o: debug.h
emalloc.o: emalloc.h
emalloc.o: stderr.h
esqlcver.o: esql_ius.h
esqlcver.o: esqlc.h
esqlcver.o: esqltype.h
esqlcver.o: esqlutil.h
estrdup.o: emalloc.h
getopt.o: getopt.h
ixblob.o: esql_ius.h
ixblob.o: esqlc.h
ixblob.o: esqltype.h
ixblob.o: ixblob.h
mkpath.o: emalloc.h
mkpath.o: jlss.h
selmultiblob.o: connblob.h
selmultiblob.o: esql_ius.h
selmultiblob.o: esqlc.h
selmultiblob.o: esqltype.h
selmultiblob.o: esqlutil.h
selmultiblob.o: getopt.h
selmultiblob.o: ixblob.h
selmultiblob.o: jlss.h
selmultiblob.o: stderr.h
sqltype.o: esql_ius.h
sqltype.o: esqlc.h
sqltype.o: esqltype.h
sqltype.o: esqlutil.h
stderr.o: stderr.h

