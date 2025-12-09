#   @(#)$Id: Install.mk,v 2010.1 2010/04/28 04:40:13 jleffler Exp $
#
#   @(#)SQLCMD Version 90.02 (2016-07-28)
#
#######################
# Distribution System #
#######################

BASENAME = sqlcmd

FILES.bod = \
	${BASENAME}.bod \
	${BASENAME}.ins \
	${BASENAME}.lst \
	jlss \
	mkbod

BOD:	all ${FILES.bod} .FORCE
	mkbod . $@ ${BASENAME}.bod
