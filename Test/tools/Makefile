# @(#)$Id: Makefile,v 1.4 2008/07/15 16:35:13 jleffler Exp $
#
# Makefile for SQLCMD Test Tools (STT)

# All SQLCMD Test Tools have a name starting 'stt_'.
# All SQLCMD Test Tools environment variables start 'STT_'.
#
# Assume that a working version of Perl 5 is available on $PATH.
# Assume that the Trace command is available, with '-L log' option.
# Assume that the environ command is available (used by Trace).
#
# Extra commands needed are:
# Trace
# environ
# pathfile      -- sometimes used (but not normally)
# rcsmunger     -- used when printing version in stt_test
# fixin         -- to fix Perl interpreter

.SUFFIXES: .pl

CP      = cp
MX      = chmod +x
FIXIN   = fixin.pl
PERL    = perl
FIXPERL = ${PERL} ${FIXIN} -s

# Rules for "compiling" Perl scripts
.pl:
	-${RM} $@
	${CP} $< $@
	${FIXPERL} $@
	${MX} $@
	${RM} $@.bak

PROGRAMS = \
	Trace \
	environ \
	pathfile \
	rcsmunger \
	stt_run \
	stt_test \
	stt_testsuite

all: ${PROGRAMS}

.DEFAULT:
	@echo "Nothing to do for $@"
