/*
@(#)File:           $RCSfile: strerror.c,v $
@(#)Version:        $Revision: 1.7 $
@(#)Last changed:   $Date: 2011/11/28 04:39:39 $
@(#)Purpose:        Return text of system error message
@(#)Author:         J Leffler
@(#)Copyright:      (C) JLSS 1990-91,1997,2002,2005,2011
@(#)Product:        SQLCMD Version 90.02 (2016-07-28)
*/

/*TABSTOP=4*/

/*
** JL 2002-04-22: Note that this use of sys_nerr and sys_errlist is not
** good.  The return type (char * rather than const char *) is mandated
** by the C standard.
*/

#include <stdio.h>
#include <errno.h>
#include <string.h>     /* strcpy() - and strerror() */

#ifndef HAVE_STRERROR
extern char *strerror(int errnum);
#endif

#ifdef DECLARE_SYS_NERR
extern int      sys_nerr;
extern char    *sys_errlist[];
#endif /* DECLARE_SYS_NERR */

#ifndef lint
/* Prevent over-aggressive optimizers from eliminating ID string */
const char jlss_id_strerror_c[] = "@(#)$Id: strerror.c,v 1.7 2011/11/28 04:39:39 jleffler Exp $";
#endif /* lint */

char *(strerror)(int errnum)
{
	static char     buff[80];

	if (errnum < 0 || errnum > sys_nerr)
		sprintf(buff, "unknown system error number %d", errnum);
	else if (errnum == 0)
		strcpy(buff, "no error (error number 0)");
	else
		strcpy(buff, sys_errlist[errnum]);
	return(buff);
}

#ifdef TEST

#define DIM(x)	(sizeof(x)/sizeof(*(x)))

static const int errors[] =
{
	0,
	EPERM,
	E2BIG,
	EEXIST,
	ERANGE,
	-1
};

int main(void)
{
	int             i;

	for (i = 0; i < DIM(errors); i++)
		printf("Error %d: %s\n", errors[i], strerror(errors[i]));
	return(0);
}

#endif	/* TEST */
