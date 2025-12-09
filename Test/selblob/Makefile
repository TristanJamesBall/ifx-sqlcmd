#	@(#)$Id: Makefile,v 1.1 2013/12/28 22:29:56 jleffler Exp $
#
#	Makefile for SQLCMD Test Suite (STT) - SELBLOB tests

TESTFILES = \
	t00basics.sh

TESTS = \
	"-q t00basics.sh"

TEST_COMMAND = stt_run

all test: ${TESTFILES}
	for test in ${TESTS}; do ${TEST_COMMAND} $$test; done

.DEFAULT:
	@echo "Nothing to do for $@"
