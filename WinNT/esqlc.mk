#	"@(#)$Id: esqlc.mk,v 1.7 2000/03/25 21:11:17 jleffler Exp $"
#
#	@(#)$Product: SQLCMD Version 90.02 (2016-07-28) $
#
#	Additional rules for Make: ESQL/C on NT

AR       = lib
ARFLAGS  = /out:$@ 
RM       = rm -f
ESQL     = $(ESQL_CMD)
ESQL_CMD = esql
EFLAGS   = -dcmdl -cc

.SUFFIXES:
.SUFFIXES: .a .obj .ec .c .y .l .s .h .sh .f

.ec:
	$(ESQL) $(EFLAGS) $(CFLAGS) $*.ec -o $@ $(LDFLAGS)
	$(RM) $*.[co]
.ec.obj:
	$(ESQL) $(EFLAGS) $(CFLAGS) -c $*.ec
	$(RM) $*.c
.ec.c:
	$(ESQL) $(EFLAGS) $(CFLAGS) -e $*.ec
.ec.a:
	$(ESQL) $(EFLAGS) $(CFLAGS) -c $*.ec
	$(AR) $(ARFLAGS) $@ $*.obj
	$(RM) $*.[co]
