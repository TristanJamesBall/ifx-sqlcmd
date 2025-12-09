:	"@(#)$Id: newserver.sh,v 1.2 1992/05/10 18:56:31 jl Exp $"
#
#	@(#)SQLCMD Version 90.02 (2016-07-28)
#
#	Start an SQLCMD Server.
#	Use: . newserver

SQLCMDFIFO=`pwd`/sql.$$
export SQLCMDFIFO

sqlserver
