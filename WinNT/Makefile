#	@(#)$Id: Makefile,v 2008.1 2008/05/19 03:05:26 jleffler Exp $
#
#	@(#)@PACKAGE@ Version @VERSION@
#
#	Makefile for SQLCMD/SQLUNLOAD/SQLRELOAD on NT

null:	all

include esqlc.mk

###################################################################
#            SQLCMD has been AutoConfiscated on Unix.             #
#       You should not need to manually configure Makefile        #
#    However, you may need to do so on Microsoft Windows NT.      #
#   If you know how to AutoConfiscate on NT, let me know too!     #
###################################################################

###################################################################
# CONFIGURATION SECTION -- edit in emergency only                 #
###################################################################
# Set OFLAGS, XFLAGS, RANLIB, LIBC and LIBMALLOC to suit you.     #
# Set OFILES_o = rfnmanip.obj if using ESQL/C 4.10 and you have   #
# located a copy of the file (from ESQL/C 4.00, or from libsace.a #
# or libfglgo.a)  Add malloc.obj if using a debugging malloc. Add #
# -DDEBUG_MALLOC to OFLAGS if your malloc package provides a      #
# routine dump_malloc() to print the current memory usage.  You   #
# can define -Ddump_malloc=xxyyzz if there is a similar routine   #
# to dump_malloc (a routine with no arguments and no return       #
# value) but by a different name.                                 #
# Note: INCDIRS should be $(INFORMIXDIR)/incl for version 4.1x    #
# Note: if you do not have ESQL/C installed but you do have I4GL, #
#       you need to simulate the esql script.  The esql.sh script #
#       can be used as a starting point, but the library name it  #
#       specifies is probably wrong.  It should be upgraded to    #
#       know about the INFORMIXC environment variable.            #
###################################################################

#CC        = gcc -ansi -pedantic -Wall -Wmissing-prototypes -Wstrict-prototypes -Wshadow
#CC        = cc -g -DDEBUG
OFLAGS    = -g -DDEBUG_MALLOC
OFLAGS    = -O2
XFLAGS    = -v
XFLAGS    =

CD            = cd
LN            = COPY
PREFIX        = c:\usr\sqlcmd
BINDIR        = $(PREFIX)\bin
ETCDIR        = $(PREFIX)\etc
MANTOP        = $(PREFIX)\man
MANDIR        = $(MANTOP)\man1
INSTALL_PROG  = COPY
INSTALL_DATA  = COPY
CC            = cl
ESQLC_VERSION = 300
ESQLC_VERSTR  = 3.00.TC1
ESQLC_INCLDIR = $(INFORMIXDIR)\incl\esql
ESQLC_AIXLOCT = 
INFORMIXDIR   = C:\Progra~1\IBM\Informix\Client-SDK
HAVE_CONFIG_H = -DHAVE_CONFIG_H
RANLIB        = echo
YACC          = echo
OFILES_o      = getopt.obj
CP            = COPY
RM            = DEL
RM_F          = DEL

# /W3 is probably a good idea (but clean compiles are rather hard to achieve).
# /WX means any warning is treated as an error - very forceful.
# Override with UFLAGS="/MD" on the nmake command line.
UFLAGS    = /MD /W3 /WX # User flags for C compiler; always overridable on command line
PFLAGS    = $(HAVE_CONFIG_H) $(ESQLC_AIXLOCT)
VERSION   = -DESQLC_VERSION=$(ESQLC_VERSION)
INCDIRS   = -I. -I$(ESQLC_INCLDIR)
LIBSQLCMD = libjlss.lib
STRIP     = #-s
AUXLIBS   = 
LDFLAGS   = $(LIBSQLCMD) $(AUXLIBS) $(STRIP)
CFLAGS    = $(UFLAGS) $(VERSION) $(INCDIRS) $(OFLAGS) $(XFLAGS) $(PFLAGS)

###################################################################
# END OF CONFIGURATION SECTION                                    #
###################################################################

APPBLOB     = appblob.exe
HISTDUMP    = histdump.exe
HISTTEST    = histtest.exe
INSBLOB     = insblob.exe
MKPROC      = mkproc.exe
MKPROC_1    = mkproc.1
MULTIBLOB   = selmultiblob.exe
SELBLOB     = selblob.exe
SQLCMD      = sqlcmd.exe
SQLCMD_1    = sqlcmd.1
SQLRELOAD   = sqlreload.exe
SQLUNLOAD   = sqlunload.exe
SQLUPLOAD   = sqlupload.exe
SQLUPLOAD_1 = sqlupload.1
UPDBLOB     = updblob.exe
UPDBLOB_1   = updblob.1

.PRECIOUS:	$(LIBSQLCMD)

PROGRAMS = \
		$(APPBLOB) \
		$(HISTDUMP) \
		$(HISTTEST) \
		$(INSBLOB) \
		$(MKPROC) \
		$(MULTIBLOB) \
		$(SELBLOB) \
		$(SQLCLEAN) \
		$(SQLCMD) \
		$(SQLRELOAD) \
		$(SQLUNLOAD) \
		$(SQLUPLOAD) \
		$(UPDBLOB)

LINKED_PROGRAMS = \
		$(APPBLOB) \
		$(HISTDUMP) \
		$(HISTTEST) \
		$(INSBLOB) \
		$(MKPROC) \
		$(MULTIBLOB) \
		$(SELBLOB) \
		$(SQLCMD) \
		$(SQLUPLOAD) \
		$(UPDBLOB)

FILES_map = $(LINKED_PROGRAMS:.exe=.map)
FILES_lnk = $(LINKED_PROGRAMS:.exe=.lnk)

SQLCMD_o = \
		connect.obj \
		connecty.obj \
		sqlcmd.obj \
		sqlinfo.obj \
		sqlver.obj

SQLUPLOAD_o = \
		guesser.obj \
		sqlupload.obj \
		sqlver.obj \
		upload.obj

APPBLOB_o   = appblob.obj connblob.obj
HISTDUMP_o  = histdump.obj
HISTTEST_o  = histtest.obj
INSBLOB_o   = insblob.obj connblob.obj
MKPROC_o    = mkproc.obj
MULTIBLOB_o = selmultiblob.obj connblob.obj
SELBLOB_o   = selblob.obj connblob.obj
UPDBLOB_o   = updblob.obj connblob.obj
 
LIBSQLCMD_o = \
		$(OFILES_o) \
		basedigit.obj \
		basename.obj \
		chkfifo.obj \
		chktty.obj \
		chrcstrlit.obj \
		cistrcmp.obj \
		conninfo.obj \
		context.obj \
		cstrlitchr.obj \
		cstrlitstr.obj \
		datefmt.obj \
		debug.obj \
		decfix.obj \
		decsci.obj \
		decsetexp.obj \
		describe.obj \
		dumpblob.obj \
		dumpdec.obj \
		dumpdtime.obj \
		dumpintvl.obj \
		dumpsqlca.obj \
		dumpsqlda.obj \
		dumpvalue.obj \
		efopen.obj \
		emalloc.obj \
		epopen.obj \
		esqlcver.obj \
		esqlver.obj \
		estrdup.obj \
		fcopy.obj \
		filelock.obj \
		getline.obj \
		getopt.obj \
		getsubopt.obj \
		history.obj \
		ifmxdec.obj \
		ilog10.obj \
		internal.obj \
		iustoken.obj \
		ixblob.obj \
		jtypes.obj \
		kludge.obj \
		lduint4.obj \
		memory.obj \
		mkpath.obj \
		nstrcpy.obj \
		nvstrcpy.obj \
		output.obj \
		quotify.obj \
		readcmd.obj \
		readload.obj \
		reload.obj \
		skip.obj \
		sqlconn.obj \
		sqlerr.obj \
		sqlownobj.obj \
		sqlprint.obj \
		sqlquote.obj \
		sqlsignal.obj \
		sqlstmt.obj \
		sqltoken.obj \
		sqltype.obj \
		sqlunquote.obj \
		stderr.obj \
		strcstrlit.obj \
		strdotfill.obj \
		strlower.obj \
		strprefix.obj \
		strupper.obj \
		stuint4.obj \
		timer.obj \
		tokencmp.obj \
		vstrcpy.obj

FILES_c = \
		basedigit.c \
		basename.c \
		chkfifo.c \
		chktty.c \
		chrcstrlit.c \
		cistrcmp.c \
		conninfo.c \
		context.c \
		cstrlitchr.c \
		cstrlitstr.c \
		datefmt.c \
		debug.c \
		decfix.c \
		decsci.c \
		decsetexp.c \
		efopen.c \
		emalloc.c \
		epopen.c \
		esqlver.c \
		estrdup.c \
		fcopy.c \
		filelock.c \
		getline.c \
		getopt.c \
		getsubopt.c \
		guesser.c \
		histdump.c \
		history.c \
		ifmxdec.c \
		ilog10.c \
		internal.c \
		iustoken.c \
		jtypes.c \
		kludge.c \
		lduint4.c \
		memory.c \
		mkpath.c \
		nstrcpy.c \
		nvstrcpy.c \
		output.c \
		quotify.c \
		readcmd.c \
		readload.c \
		skip.c \
		sqlcmd.c \
		sqlerr.c \
		sqlprint.c \
		sqlquote.c \
		sqlsignal.c \
		sqltoken.c \
		sqlunquote.c \
		sqlupload.c \
		sqlver.c \
		stderr.c \
		strcstrlit.c \
		strdotfill.c \
		strlower.c \
		strprefix.c \
		strupper.c \
		stuint4.c \
		timer.c \
		tokencmp.c \
		vstrcpy.c

FILES_ec = \
		connect.ec \
		describe.ec \
		dumpblob.ec \
		dumpdec.ec \
		dumpdtime.ec \
		dumpintvl.ec \
		dumpsqlca.ec \
		dumpsqlda.ec \
		dumpvalue.ec \
		esqlcver.ec \
		ixblob.ec \
		mkproc.ec \
		reload.ec \
		sqlconn.ec \
		sqlinfo.ec \
		sqlownobj.ec \
		sqlstmt.ec \
		sqltype.ec \
		updblob.ec \
		upload.ec

FILES_c_ = $(FILES_ec:.ec=.c_)

FILES_y = connecty.y

all:	$(PROGRAMS)
	-dir $(PROGRAMS)

$(LIBSQLCMD):	$(LIBSQLCMD_o)
	$(AR) $(ARFLAGS) $(LIBSQLCMD_o)
	$(RANLIB) $@

$(SQLCMD):	$(SQLCMD_o) $(LIBSQLCMD)
	$(ESQL) -o $@ $(SQLCMD_o) $(LDFLAGS)

$(SQLUPLOAD):	$(SQLUPLOAD_o) $(LIBSQLCMD)
	$(ESQL) -o $@ $(SQLUPLOAD_o) $(LDFLAGS)

$(MKPROC):	$(MKPROC_o) $(LIBSQLCMD)
	$(ESQL) -dcmdl -o $@ $(MKPROC_o) $(LDFLAGS)

$(APPBLOB):	$(APPBLOB_o) $(LIBSQLCMD)
	$(ESQL) -o $@ $(APPBLOB_o) $(LDFLAGS)

$(INSBLOB):	$(INSBLOB_o) $(LIBSQLCMD)
	$(ESQL) -o $@ $(INSBLOB_o) $(LDFLAGS)

$(SELBLOB):	$(SELBLOB_o) $(LIBSQLCMD)
	$(ESQL) -o $@ $(SELBLOB_o) $(LDFLAGS)

$(MULTIBLOB):	$(MULTIBLOB_o) $(LIBSQLCMD)
	$(ESQL) -o $@ $(MULTIBLOB_o) $(LDFLAGS)

$(UPDBLOB):	$(UPDBLOB_o) $(LIBSQLCMD)
	$(ESQL) -o $@ $(UPDBLOB_o) $(LDFLAGS)

$(SQLRELOAD) $(SQLUNLOAD):	$(SQLCMD)
	$(RM) $@ && $(LN) $? $@

$(HISTTEST):	$(HISTTEST_o) $(LIBSQLCMD)
	$(ESQL) -o $@ $(HISTTEST_o) $(LDFLAGS)

$(HISTDUMP):	$(HISTDUMP_o) $(LIBSQLCMD)
	$(CC) -o $@ $(HISTDUMP_o) $(LDFLAGS)

connecty.c: connecty.c.std
	$(RM) $@
	$(CP) $? $@

connecty: connecty.y
	$(YACC) $(YFLAGS) $@.y
	$(ESQL) -o $@ -DTEST $(CFLAGS) y.tab.c $(LDFLAGS)
	$(RM) y.tab.[co]

sqlver.obj: sqlver.c
	$(CC) $(CFLAGS) -DESQLC_VERSTR="$(ESQLC_VERSTR)" -c sqlver.c

#############################
# Include file dependencies #
#############################

include Depend.mk

# These commands probably only work if $(PREFIX) is an absolute path
install:	all
	if not exist $(PREFIX) mkdir $(PREFIX)
	if not exist $(BINDIR) mkdir $(BINDIR)
	if not exist $(MANTOP) mkdir $(MANTOP)
	if not exist $(MANDIR) mkdir $(MANDIR)
	$(INSTALL_PROG) $(HISTDUMP)    $(BINDIR)\$(HISTDUMP)
	$(INSTALL_PROG) $(MKPROC)      $(BINDIR)\$(MKPROC)
	$(INSTALL_PROG) $(SQLCMD)      $(BINDIR)\$(SQLCMD)
	$(INSTALL_PROG) $(UPDBLOB)     $(BINDIR)\$(UPDBLOB)
	$(INSTALL_PROG) $(SQLUPLOAD)   $(BINDIR)\$(SQLUPLOAD)
	$(INSTALL_DATA) $(MKPROC_1)    $(MANDIR)\$(MKPROC_1)
	$(INSTALL_DATA) $(SQLCMD_1)    $(MANDIR)\$(SQLCMD_1)
	$(INSTALL_DATA) $(UPDBLOB_1)   $(MANDIR)\$(UPDBLOB_1)
	$(INSTALL_DATA) $(SQLUPLOAD_1) $(MANDIR)\$(SQLUPLOAD_1)
	$(CD) $(BINDIR) && if exist $(SQLRELOAD) $(RM) $(SQLRELOAD)
	$(CD) $(BINDIR) && $(LN) $(SQLCMD) $(SQLRELOAD)
	$(CD) $(BINDIR) && if exist $(SQLUNLOAD) $(RM) $(SQLUNLOAD)
	$(CD) $(BINDIR) && $(LN) $(SQLCMD) $(SQLUNLOAD)

#######################
# Distribution System #
#######################

.FORCE:

###########
# Cleanup #
###########
clean:
	$(RM) $(PROGRAMS) $(FILES_map) $(FILES_lnk)
	$(RM) $(LIBSQLCMD) *.obj $(FILES_c_)

realclean:	clean
	$(RM) Makefile config.h config.log config.status config.cache esqlinfo.h
