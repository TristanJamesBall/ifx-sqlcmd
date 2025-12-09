#	@(#)$Id: Makefile,v 2013.1 2013/12/28 22:30:44 jleffler Exp $
#
#	Makefile for SQLCMD Test Suite

.SILENT:

SFLAG = -s

TEST.DIRECTORIES = \
	sqlcmd \
	selblob

#	sqlreload \
#	sqlunload \
#	sqlclient \
#	sqlupload

DIRECTORIES = tools ${TEST.DIRECTORIES}

all:
	cd tools; ${MAKE}
	tools/stt_testsuite ${MAKE} run-tests

test:	all

run-tests:
	for d in ${TEST.DIRECTORIES}; do (echo "-> $$d"; cd $$d; ${MAKE} ${SFLAG} test) done 

.DEFAULT:
	cd tools; ${MAKE}
	tools/stt_testsuite ${MAKE} $@
