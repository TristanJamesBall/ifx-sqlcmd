#	@(#)$Id: Makefile,v 1.6 2008/07/14 03:27:17 jleffler Exp $
#
#	Makefile for SQLCMD Test Suite (STT) - SQLCMD tests

TESTFILES = \
	t00version.sh \
	t01mutexopt.sh \
	t02varchar.sh \
	t03boolean.sh \
	t04lvarchar.sh \
	t05text.sh

TESTS = \
	"-v t00version.sh" \
	"-q t01mutexopt.sh" \
	"-q t02varchar.sh" \
	"-q t03boolean.sh" \
	"-q t04lvarchar.sh" \
	"-q t05text.sh"

TEST_COMMAND = stt_run

all test: ${TESTFILES}
	for test in ${TESTS}; do ${TEST_COMMAND} $$test; done

.DEFAULT:
	@echo "Nothing to do for $@"
