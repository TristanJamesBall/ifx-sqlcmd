/*
@(#)File:           $RCSfile: sqlprint.c,v $
@(#)Version:        $Revision: 2005.1 $
@(#)Last changed:   $Date: 2005/06/22 23:50:51 $
@(#)Purpose:        Report database generated error
@(#)Author:         J Leffler
@(#)Copyright:      (C) JLSS 1998-99,2003-05
@(#)Product:        SQLCMD Version 90.02 (2016-07-28)
*/

/*TABSTOP=4*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <string.h>
#include <stdio.h>
#include <sqlca.h>
#include <sqlhdr.h>
#include "esqlutil.h"

static const char errmsg_notfound[] = "<<error message text not found>>\n";

#ifndef lint
static const char rcs[] = "@(#)$Id: sqlprint.c,v 2005.1 2005/06/22 23:50:51 jleffler Exp $";
#endif

static int sql_getmsg(long errnum, char *buffer, size_t buflen)
{
	int rc;
#ifdef ESQLC_RGETLMSG
	int	msglen;
	rc =  rgetlmsg(errnum, buffer, buflen, &msglen);
#else
	rc =  rgetmsg(errnum, buffer, buflen);
#endif /* ESQLC_RGETLMSG */
	return(rc);
}

static char *sql_msgstr(char *buffer, size_t buflen, char const *tag, long errnum, char const *arg)
{
	char            locbuf[512];
	const char     *locptr;

	locptr = locbuf;
	if (sql_getmsg(errnum, locbuf, sizeof(locbuf)) < 0)
		locptr = errmsg_notfound;
	sprintf(buffer, "%s %ld: ", tag, errnum);
	sprintf(buffer + strlen(buffer), locptr, arg);
	return(buffer + strlen(buffer));
}

void sql_formaterror(char *buffer, size_t buflen)
{
	if (sqlca.sqlcode == 0 && sqlca.sqlerrd[1] == 0)
	{
		sprintf(buffer, "SQL 0: No error\n");
	}
	else
	{
		char *end = sql_msgstr(buffer, buflen, "SQL", sqlca.sqlcode, sqlca.sqlerrm);
		if (sqlca.sqlerrd[1] != 0)
			end = sql_msgstr(end, buflen - (end - buffer), "ISAM", sqlca.sqlerrd[1], sqlca.sqlerrm);
	}
}

/* Report database generated error from global sqlca record */
void            sql_printerror(FILE *fp)
{
	char            buffer[512];

	fflush(0);	/* Flush all pending i/o */
	sql_formaterror(buffer, sizeof(buffer));
	fputs(buffer, fp);
	fflush(fp);
}

#ifdef TEST

#include <stdlib.h>
#include "stderr.h"

static void process_error(char const *arg)
{
	long	sql;
	long	isam = 0;
	char *end;

	sql = strtol(arg, &end, 0);
	if (end == arg || (*end != '\0' && *end != ':'))
		err_error2("invalid SQL error number", arg);
	if (*end == ':')
	{
		arg = end + 1;
		isam = strtol(arg, &end, 0);
		if (end == arg || *end != '\0')
			err_error2("invalid ISAM error number", arg);
	}
	sqlca.sqlcode = sql;
	sqlca.sqlerrd[1] = isam;
	strcpy(sqlca.sqlerrm, "<<STRING ARGUMENT>>");
	sql_printerror(stdout);
}

int main(int argc, char **argv)
{
	int i;

	err_setarg0(argv[0]);
	if (argc < 2)
		err_usage("sql[:isam] ...");

	for (i = 1; i < argc; i++)
		process_error(argv[i]);

	return(0);
}

#endif /* TEST */
