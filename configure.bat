rem @(#)$Id: configure.bat,v 1.8 2008/05/19 00:48:48 jleffler Exp $
rem Install NT configuration files for SQLCMD etc

if not exist esqlc.mk.unix rename esqlc.mk esqlc.mk.unix
copy WinNT\Depend.mk .
copy WinNT\Makefile .
copy WinNT\config.h .
copy WinNT\esqlc.mk .
copy WinNT\esqlinfo.h .
move connecty.y connecty.xxx
copy connecty.c.std connecty.c

echo off
echo "Sorry, I don't have a good way to autoconfiscate on NT"
echo "Manually edit the Makefile - expecially the ESQL/C version info"
