#	"@(#)$Id: esqlc.mk,v 1.6 1999/10/04 05:02:54 jleffler Exp $"
#
#	@(#)SQLCMD Version 90.02 (2016-07-28)
#
#	Additional rules for Make: ESQL/C

AR       = ar
ARFLAGS  = rv
RM       = rm -f
ESQL     = INFORMIXC="${CC} -v -static -O3" ${ESQL_CMD} 
ESQL_CMD = esql 
EFLAGS   =

.SUFFIXES:
.SUFFIXES: .a .o .ec .ec~ .c .c~ .y .y~ .l .l~ .s .s~ .h .h~ 
.SUFFIXES: .sh .sh~ .f .f~ 

.ec:
	${ESQL} ${EFLAGS} ${CFLAGS} $< -o $@ ${LDFLAGS}
	${RM} $*.[co]
.ec~:
	${GET} ${GFLAGS} -p $< >$*.ec
	${ESQL} ${EFLAGS} ${CFLAGS} $*.ec -o $@ ${LDFLAGS}
	${RM} $*.ec $*.[co]
.ec.o:
	${ESQL} ${EFLAGS} ${CFLAGS} -c $<
	${RM} $*.c
.ec~.o:
	${GET} ${GFLAGS} -p $< >$*.ec
	${ESQL} ${EFLAGS} ${CFLAGS} -c $*.ec
	${RM} $*.ec $*.c
.ec.c:
	${ESQL} ${EFLAGS} ${CFLAGS} -e $*.ec
.ec~.c:
	${GET} ${GFLAGS} -p $< >$*.ec
	${ESQL} ${EFLAGS} ${CFLAGS} -e $*.ec
	${RM} $*.ec
.ec~.ec:
	${GET} ${GFLAGS} -p $< >$*.ec
.ec.a:
	${ESQL} ${EFLAGS} ${CFLAGS} -c $<
	${AR} ${ARFLAGS} $@ $*.o
	${RM} $*.[co]
.ec~.a:
	${GET} ${GFLAGS} -p $< >$*.ec
	${ESQL} ${EFLAGS} ${CFLAGS} -c $*.ec
	${AR} ${ARFLAGS} $@ $*.o
	${RM} $*.ec $*.[co]
