/*
@(#)File:           $RCSfile: internal.c,v $
@(#)Version:        $Revision: 2014.1 $
@(#)Last changed:   $Date: 2014/07/28 03:05:10 $
@(#)Purpose:        Internal Commands for SQLCMD
@(#)Author:         J Leffler
@(#)Copyright:      (C) JLSS 1992-93,1995-2012,2014
@(#)Product:        SQLCMD Version 90.02 (2016-07-28)
*/

/*TABSTOP=4*/

/* -- Include Files */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>     /* sleep(), unlink() */
#else
#ifndef _MSC_VER    /* Used by MS Visual Studio in <sys/stat.h> */
/* JL 2008-05-18: Not clear why these declarations caused trouble, but they did */
extern unsigned int sleep(unsigned int);
extern int unlink(const char *file);
#endif /* _MSC_VER */
#endif /* HAVE_UNISTD_H */

#define SECS_IN_ONE_YEAR    (365.2422 * 86400)

#include "connect.h"
#include "context.h"
#include "emalloc.h"
#include "esqlc.h"
#include "esqlutil.h"
#include "history.h"
#include "internal.h"
#include "jlsstools.h"
#include "esnprintf.h"
#include "kludge.h"
#include "openfile.h"
#include "readcmd.h"
#include "sqlcmd.h"
#include "sqlerr.h"
#include "sqltoken.h"
#include "stderr.h"
#include "timer.h"
#include "ixblob.h"     /* sql_dbtemp() */

/* -- Constant Definitions */

#ifndef DEF_EDITOR
#define DEF_EDITOR "vi"
#endif /* DEF_EDITOR */

#define ICMD_FULLCMD    0x01    /* Pass whole command to parser function */
#define ICMD_TAILCMD    0x02    /* Pass second word onwards to parser function */
#define ICMD_NOHISTORY  0x04    /* Do not record command in history */
#define ICMD_EXTERNAL   0x08    /* Record benchmark attributes for these commands */
#define ICMD_MANGLE     0x10    /* Record modified command in history - CONNECT */

#define TMPFILE_TEMPLATE    "sqlcmd.XXXXXX"

#define MIN(a,b)    (((a)<(b))?(a):(b))

enum { SQL_CMDBUFSIZE    = (4*65536) };

/* -- Type Definitions */

/* Information about a command */
typedef struct Command
{
    const char     *name;
    void          (*function)(char *);
    char           *arg;
    int             flags;
}               Command;

/* -- Declarations */

static void do_adjust(char *s);
static void do_benchmark(char *s);
static void do_blobdir(char *s);
static void do_clock(char *s);
static void do_context(char *s);
static void do_continue(char *s);
static void do_date(char *s);
static void do_dbnames(char *s);
static void do_delim(char *s);
static void do_echo(char *s);
static void do_edit(char *s);
static void do_eor(char *s);
static void do_erase(char *s);
static void do_errmsg(char *s);
static void do_error(char *s);
static void do_escape(char *s);
static void do_exit(char *s);
static void do_format(char *s);
static void do_heading(char *s);
static void do_hexmode(char *s);
static void do_help(char *s);
static void do_history(char *s);
static void do_ibase(char *s);
static void do_input(char *s);
static void do_invalid(char *s);
static void do_list(char *s);
static void do_load_op(char *s);
static void do_notallowed(char *s);
static void do_output(char *s);
static void do_qlimit(char *s);
static void do_quote(char *s);
static void do_rerun(char *s);
static void do_shell(char *s);
static void do_silence(char *s);
static void do_single(char *s);
static void do_sleep(char *s);
static void do_sqlexit(char *s);
static void do_sqlstart(char *s);
static void do_time(char *s);
static void do_trace(char *s);
static void do_transize(char *s);
static void do_types(char *s);
static void do_verbose(char *s);
static void do_version(char *s);
static void do_xml(char *s);

/**INDENT-OFF**/
/* Maintain in sorted order */
static const Command icmds[] =
{
    {   "adjust",     do_adjust,      (char *)0,  ICMD_TAILCMD|ICMD_NOHISTORY },
    {   "b",          do_exit,        (char *)0,  ICMD_TAILCMD|ICMD_NOHISTORY },
    {   "benchmark",  do_benchmark,   (char *)0,  ICMD_TAILCMD    },
    {   "blobdir",    do_blobdir,     (char *)0,  ICMD_TAILCMD    },
    {   "bye",        do_exit,        (char *)0,  ICMD_TAILCMD|ICMD_NOHISTORY },
    {   "clock",      do_clock,       (char *)0,  ICMD_TAILCMD    },
    {   "connect",    do_connect,     (char *)0,  ICMD_FULLCMD|ICMD_EXTERNAL|ICMD_MANGLE  },
    {   "context",    do_context,     (char *)0,  ICMD_TAILCMD    },
    {   "continue",   do_continue,    (char *)0,  ICMD_TAILCMD    },
    {   "d",          do_erase,       (char *)0,  ICMD_TAILCMD|ICMD_NOHISTORY },
    {   "date",       do_date,        (char *)0,  ICMD_TAILCMD    },
    {   "dbnames",    do_dbnames,     (char *)0,  ICMD_TAILCMD    },
    {   "delim",      do_delim,       (char *)0,  ICMD_TAILCMD    },
    {   "delimit",    do_delim,       (char *)0,  ICMD_TAILCMD    },
    {   "delimiter",  do_delim,       (char *)0,  ICMD_TAILCMD    },
    {   "disconnect", do_disconn,     (char *)0,  ICMD_FULLCMD|ICMD_EXTERNAL  },
    {   "e",          do_exit,        (char *)0,  ICMD_TAILCMD|ICMD_NOHISTORY },
    {   "echo",       do_echo,        (char *)0,  ICMD_TAILCMD    },
    {   "edit",       do_edit,        (char *)0,  ICMD_TAILCMD|ICMD_NOHISTORY },
    {   "eor",        do_eor,         (char *)0,  ICMD_TAILCMD    },
    {   "erase",      do_erase,       (char *)0,  ICMD_TAILCMD|ICMD_NOHISTORY },
    {   "errmsg",     do_errmsg,      (char *)0,  ICMD_TAILCMD    },
    {   "error",      do_error,       (char *)0,  ICMD_TAILCMD    },
    {   "escape",     do_escape,      (char *)0,  ICMD_TAILCMD    },
    {   "exit",       do_exit,        (char *)0,  ICMD_TAILCMD|ICMD_NOHISTORY },
    {   "format",     do_format,      (char *)0,  ICMD_TAILCMD    },
    {   "heading",    do_heading,     (char *)0,  ICMD_TAILCMD    },
    {   "headings",   do_heading,     (char *)0,  ICMD_TAILCMD    },
    {   "help",       do_help,        (char *)0,  ICMD_TAILCMD    },
    {   "hexmode",    do_hexmode,     (char *)0,  ICMD_TAILCMD    },
    {   "history",    do_history,     (char *)0,  ICMD_TAILCMD    },
    {   "ibase",      do_ibase,       (char *)0,  ICMD_TAILCMD    },
    {   "info",       sql_info,       (char *)0,  ICMD_FULLCMD|ICMD_EXTERNAL  },
    {   "input",      do_input,       (char *)0,  ICMD_TAILCMD    },
    {   "invalid",    do_invalid,     (char *)0,  ICMD_FULLCMD    },
    {   "l",          do_list,        (char *)0,  ICMD_TAILCMD|ICMD_NOHISTORY },
    {   "list",       do_list,        (char *)0,  ICMD_TAILCMD|ICMD_NOHISTORY },
    {   "load",       do_load_op,     (char *)0,  ICMD_FULLCMD|ICMD_EXTERNAL  },
    {   "notallowed", do_notallowed,  (char *)0,  ICMD_FULLCMD    },
    {   "output",     do_output,      (char *)0,  ICMD_TAILCMD    },
    {   "q",          do_exit,        (char *)0,  ICMD_TAILCMD|ICMD_NOHISTORY },
    {   "qlimit",     do_qlimit,      (char *)0,  ICMD_TAILCMD    },
    {   "querylimit", do_qlimit,      (char *)0,  ICMD_TAILCMD    },
    {   "quit",       do_exit,        (char *)0,  ICMD_TAILCMD|ICMD_NOHISTORY },
    {   "quote",      do_quote,       (char *)0,  ICMD_TAILCMD    },
    {   "r",          do_rerun,       (char *)0,  ICMD_TAILCMD|ICMD_NOHISTORY },
    {   "reload",     do_load_op,     (char *)0,  ICMD_FULLCMD|ICMD_EXTERNAL  },
    {   "rerun",      do_rerun,       (char *)0,  ICMD_TAILCMD|ICMD_NOHISTORY },
    {   "set",        do_setconn,     (char *)0,  ICMD_FULLCMD|ICMD_EXTERNAL  },
    {   "shell",      do_shell,       (char *)0,  ICMD_TAILCMD    },
    {   "silence",    do_silence,     (char *)0,  ICMD_TAILCMD    },
    {   "single",     do_single,      (char *)0,  ICMD_TAILCMD    },
    {   "sleep",      do_sleep,       (char *)0,  ICMD_TAILCMD    },
    {   "sql",        sql_command,    (char *)0,  ICMD_TAILCMD|ICMD_EXTERNAL  },
    {   "sqlexit",    do_sqlexit,     (char *)0,  ICMD_TAILCMD|ICMD_EXTERNAL  },
    {   "sqlstart",   do_sqlstart,    (char *)0,  ICMD_TAILCMD|ICMD_EXTERNAL  },
    {   "time",       do_time,        (char *)0,  ICMD_TAILCMD    },
    {   "trace",      do_trace,       (char *)0,  ICMD_TAILCMD    },
    {   "trans",      do_transize,    (char *)0,  ICMD_TAILCMD    },
    {   "transize",   do_transize,    (char *)0,  ICMD_TAILCMD    },
    {   "type",       do_types,       (char *)0,  ICMD_TAILCMD    },
    {   "types",      do_types,       (char *)0,  ICMD_TAILCMD    },
    {   "unload",     do_load_op,     (char *)0,  ICMD_FULLCMD|ICMD_EXTERNAL  },
    {   "v",          do_edit,        (char *)0,  ICMD_TAILCMD|ICMD_NOHISTORY },
    {   "verbose",    do_verbose,     (char *)0,  ICMD_TAILCMD    },
    {   "version",    do_version,     (char *)0,  ICMD_TAILCMD|ICMD_NOHISTORY },
    {   "vi",         do_edit,        (char *)0,  ICMD_TAILCMD|ICMD_NOHISTORY },
    {   "view",       do_edit,        (char *)0,  ICMD_TAILCMD|ICMD_NOHISTORY },
    {   "x",          do_exit,        (char *)0,  ICMD_TAILCMD|ICMD_NOHISTORY },
    {   "xml",        do_xml,         (char *)0,  ICMD_TAILCMD    },
};
/**INDENT-ON**/

/* Maybe use structure with 2 strings, one for command, one for explanation. */
/* Drop the space and dash - double up on synonyms, etc */
static const char sqlcmd_help[] =
    "adjust         - change number of records in history log (argument: number of records)\n"
    "b              - synonym for exit (bye)\n"
    "benchmark      - control benchmark mode (argument: on, off)\n"
    "blobdir        - set blob directory - not operational (argument: name of directory)\n"
    "bye            - synonym for exit\n"
    "clock          - control a software timer (argument: start, stop)\n"
    "connect        - ESQL/C CONNECT statement\n"
    "context        - print details about current SQLCMD context\n"
    "continue       - primitive error control (argument: push, pop, on, off)\n"
    "date           - change date format\n"
    "delim          - synonym for delimiter\n"
    "delimit        - synonym for delimiter\n"
    "delimiter      - change field delimiter\n"
    "disconnect     - ESQL/C DISCONNECT statement\n"
    "e              - synonym for exit\n"
    "echo           - print string that follows on output\n"
    "edit           - synonym for view\n"
    "eor            - change end of record string\n"
    "errmsg         - print string that follows on error\n"
    "error          - change error file\n"
    "escape         - change escape character\n"
    "exit           - terminate SQLCMD\n"
    "format         - output format for SELECT statements (argument: select, unload, csv, quote, xml)\n"
    "heading        - synonym for headings\n"
    "headings       - print column name headings (argument: push, pop, on, off)\n"
    "hexmode        - DB-Access hex (-X) mode\n"
    "help           - print this information\n"
    "history        - change history logging (argument: push, pop, on, off)\n"
    "ibase          - change input base (argument: 0..16)\n"
    "info           - print information about database: see 'info help'\n"
    "input          - change input file (starts new context)\n"
    "l              - synonym for list\n"
    "list           - list recent SQL commands\n"
    "load           - extended DB-Access LOAD statement\n"
    "output         - change output file\n"
    "q              - synonym for exit (quit)\n"
    "qlimit         - synonym for querylimit\n"
    "querylimit     - change limit on number of rows returned by a query\n"
    "quit           - exit program (optional argument: exit status 0..255)\n"
    "quote          - change quote character (for CSV, quote format)\n"
    "r              - synonym for rerun\n"
    "reload         - extended DB-Access LOAD statement with implicit transactions\n"
    "rerun          - rerun recent SQL statements\n"
    "set connection - ESQL/C SET CONNECTION statement\n"
    "shell          - run following string as shell command\n"
    "silence        - change silence mode (argument: push, pop, on, off)\n"
    "sleep          - sleep for given time\n"
    "sql            - execute following material as an SQL statement - overrides internal interpretation\n"
    "sqlexit        - terminate the server connection\n"
    "sqlstart       - initiate a server connection\n"
    "time           - print current date and time\n"
    "trace          - change trace mode (argument: push, pop, on, off)\n"
    "trans          - synonym for transize\n"
    "transize       - change size of implicit transaction in RELOAD\n"
    "type           - synonym for types\n"
    "types          - print column type information (argument: push, pop, on, off)\n"
    "unload         - extended DB-Access UNLOAD statement\n"
    "v              - synonym for view\n"
    "verbose        - change verbose mode (argument: push, pop, on, off)\n"
    "version        - print version information on output\n"
    "vi             - synonym for view\n"
    "view           - edit selected recent SQL commands and rerun them\n"
    "x              - synonym for exit\n"
    "xml            - change XML tags (argument: recset=settag,record=rectag,header=hdrtag)\n"
;

#ifndef lint
/* Prevent over-aggressive optimizers from eliminating ID string */
const char jlss_id_internal_c[] = "@(#)$Id: internal.c,v 2014.1 2014/07/28 03:05:10 jleffler Exp $";
#endif /* lint */

/* Print date/time on nominated file channel */
static void pr_time(FILE *fp)
{
    time_t     now;
    struct tm  tp;
    char       buffer[64];
    static const char tm_format[] = "%Y-%m-%d %H:%M:%S\n";

    now = time((time_t *)0);
    localtime_r(&now,&tp);
    strftime(buffer, sizeof(buffer), tm_format, &tp);
    fputs(buffer, fp);
}

/*
** Security concerns: do not record actual username and password in history.
** Record user "anonymous" password "anonymous" instead.
*/
static const char *mangle_username_and_password(const char *s)
{
	KLUDGE("Should reparse and mangle user name and password");
    return(s);

}

/* Record command in history log file */
static void hist_record(const char *s, int flags)
{
    long        cmdnum;
    const char *t = s;

	/* not supported yet */
    if (flags & ICMD_MANGLE) {
		return;
	}
    /*    t = mangle_username_and_password(s); */

    if ((cmdnum = hist_write(t)) >= 0)
    {
        cmd_add_to_readline_history(t);
        cmd_set_promptnum(cmdnum);
    }

    if (t != s)
        FREE(t);
}

/* Name comparison for cmd_lookup() function */
static int name_compare(const void *s1, const void *s2)
{
    const Command *c1 = (const Command *)s1;
    const Command *c2 = (const Command *)s2;
    return(cistrcmp(c1->name, c2->name));
}

/* Search for command in table of commands */
static int cmd_lookup(char *s, Command *command)
{
    Command         cmd;
    Command        *cp = 0;
    size_t          nbytes;
    char            cmdbuff[32];
    char           *t1;
    char           *t2;

    t1 = skipblanks(s);
    t2 = skipalphanum(t1);
    nbytes = t2 - t1;
    if (nbytes < sizeof(cmdbuff))
    {
        strncpy(cmdbuff, t1, nbytes);
        cmdbuff[nbytes] = '\0';
        cmd.name = cmdbuff;
        cp = (Command *)bsearch(&cmd, icmds, DIM(icmds),
                                 sizeof(Command), name_compare);
    }

    if (cp != NIL(Command *))
    {
        *command = *cp;
        if (command->flags & ICMD_FULLCMD)
            command->arg = t1;
        else if (command->flags & ICMD_TAILCMD)
            command->arg = t2;
        else
            command->arg = NIL(char *);
    }
    return((cp == NIL(Command *)) ? -1 : 0);
}

#ifdef SQLREAD

static int keywd_compare(const void *v1, const void *v2)
{
    const char *s1 = *(const char **)v1;
    const char *s2 = *(const char **)v2;
    int rv = cistrcmp(s1, s2);
    return(rv);
}

/* Copy a keyword into the buffer */
static void copy_keyword(char *buffer, size_t buflen, const char *stmt, const char **end)
{
    const char *src = stmt;
    const char *bgn;
    SQLComment  cmt;

    while ((cmt = sqlcomment(src, JLSS_ALLSQL_COMMENTS, &bgn, end)) == SQL_COMMENT || cmt == SQL_OPTIMIZERHINT)
        src = *end;

    if (*(src = iustoken(bgn, end)) != '\0' && src != *end)
        nstrcpy(buffer, MIN(buflen, *end - src + 1), src);
    else
        *buffer = '\0';
}

/* Return keyword entry in list if keyword found in list */
static const char *keyword_in_list(const char *keyword, const char **list, size_t listlen)
{
    const char *kw = (const char *)bsearch(&keyword, list, listlen, sizeof(char *), keywd_compare); /*=C++=*/
    return(kw);
}

/* sql_readonly() - is command allowed in SQLREAD mode
** SELECT, UNLOAD with SELECT (not EXECUTE), and some SET statements are
** OK, plus explicit connection management statements.
*/
static int sql_readonly(const char *stmt)
{
    static const char  keyword_set[] = "SET";
    static const char *keywords[] =
    {
        "CONNECT",
        "DISCONNECT",
        "SELECT",
        keyword_set,
        "UNLOAD",
    };
    static const char *setwords[] =
    {
        "BUFFERED",
        "COLLATION",
        "CONNECTION",
        "DATASKIP",
        "DEBUG",
        "ENCRYPTION",
        "ENVIRONMENT",
        "EXPLAIN",
        "ISOLATION",
        "LOCK",
        "LOG",
        "OPTIMIZATION",
        "PDQPRIORITY",
        "ROLE",
        "SESSION",
        "STATEMENT",
        "TRANSACTION",
    };
    const char *end;
    char        buffer[20];
    const char *kw;
    copy_keyword(buffer, sizeof(buffer), stmt, &end);
    if ((kw = keyword_in_list(buffer, keywords, DIM(keywords))) == 0)
        return 0;   /* Not a recognized statement */
    if (kw == keyword_set)
    {
        /* Found SET - what's the next key word? */
        copy_keyword(buffer, sizeof(buffer), end, &end);
        if ((kw = keyword_in_list(buffer, setwords, DIM(setwords))) == 0)
            return 0;   /* Not a permitted SET statement */
    }
    return 1;   /* Recognized and permitted statement/keyword */
}
#endif /* SQLREAD */

/* Determine how to execute the command.  */
static void sql_internal(char *stmt, Command *cp)
{
    if (*stmt == '!')
    {
        char cmd_shell[] = "shell"; /*=C++=*/
        if (cmd_lookup(cmd_shell, cp) != 0)
            error_746("cannot find built-in 'shell' command");
        cp->arg = stmt + 1;
    }
    else if (*stmt == '@')
    {
        /**
        ** JL 2001-11-15: Once upon a time, you could use a string
        ** of at symbols before a command.  Now you can't!
        */
        if (cmd_lookup(stmt+1, cp) != 0)
        {
            char cmd_invalid[] = "invalid"; /*=C++=*/
            if (cmd_lookup(cmd_invalid, cp) != 0)
                error_746("cannot find built-in 'invalid' command");
            cp->arg = stmt;
        }
    }
    else if (cmd_lookup(stmt, cp) != 0)
    {
#ifdef SQLREAD
        if (sql_readonly(stmt))
        {
#endif /* SQLREAD */
        char cmd_sql[] = "sql"; /*=C++=*/
        if (cmd_lookup(cmd_sql, cp) != 0)
            error_746("cannot find built-in 'sql' command");
        cp->arg = stmt;
#ifdef SQLREAD
        }
        else
        {
            char cmd_notallowed[] = "notallowed";   /*=C++=*/
            if (cmd_lookup(cmd_notallowed, cp) != 0)
                error_746("cannot find built-in 'notallowed' command");
            cp->arg = stmt;
        }
#endif /* SQLREAD */
    }
}

/* Decide how to execute a command (internal or SQL) */
void do_command(const char *str)
{
    Command         command = { 0 };
    Clock           clk;
    char           *cmd;
    const char     *stmt = str;

    /* Ignore any leading white space and comments in the command string */
    while (*(cmd = iustoken(stmt, &stmt)) != '\0')
    {
        /* Skip optimizer hints when they occur before any key word */
        /* KLUDGE - should be using sqlcomment? */
        if (strncmp(cmd, "{", 1) != 0 && strncmp(cmd, "--", 2) != 0 && strncmp(cmd, "/*", 2) != 0)
            break;
    }

    /* Don't process empty commands */
    if (*cmd == '\0')
        return;

    /* Determine how to execute the command */
    sql_internal(cmd, &command);

    /* Record it if required */
    if ((command.flags & ICMD_NOHISTORY) == 0 && ctxt_gethistory() == OP_ON)
        hist_record(str, command.flags);

    /* In benchmark mode, all SQL commands automatically produce a  */
    /* date stamp, trace and timing on the error output.            */
    /* In non-benchmark mode, only print trace when trace enabled.  */
    if (ctxt_getbmark() == OP_ON && (command.flags & ICMD_EXTERNAL))
    {
        pr_time(ctxt_error());
        fprintf(ctxt_error(), "+ %s\n", str);
        clk_init(&clk);
        clk_start(&clk);
    }
    else if (ctxt_gettrace() == OP_ON)
    {
        fprintf(ctxt_error(), "+ %s\n", str);
    }

    /* Execute command via indirection */
    (*command.function)(command.arg);

    /* Report elapsed time in benchmark mode */
    if (ctxt_getbmark() == OP_ON && (command.flags & ICMD_EXTERNAL))
    {
        char buffer[20];
        clk_stop(&clk);
        (void)clk_elapsed_us(&clk, buffer, sizeof(buffer));
        fprintf(ctxt_error(), "Time: %s\n", buffer);
    }

#ifdef ENABLE_SINGLE_STEP
    /* In debug (single-step) mode - wait for return before continuing */
    if (ctxt_getsingle() == OP_ON)
        hit_return_to_continue();
#endif /* ENABLE_SINGLE_STEP */
}

/* Process an input file */
void sql_filename(char *file)
{
    if (*file != '\0')
    {
        ctxt_newcontext();
        if (ctxt_setinput(file) == 0)
        {
            ctxt_sethistory(op_off);
            sql_file();
        }
        ctxt_endcontext();
    }
}

/* Parse an input file */
void sql_file(void)
{
    FILE           *fp;
    char            buffer[SQL_CMDBUFSIZE];

    fp = ctxt_input();
    while (cmd_read(fp, buffer, sizeof(buffer)) != EOF)
    {
        ctxt_setlinenum(cmd_get_linenum());
        do_command(buffer);
    }
}

/* Implement monitor mode */
void sql_monitor(char *fifo)
{
    if (chk_fifo(fifo) != 0)
        cmd_error(E_NOTFIFO, fifo);

    /* Monitor -- device is assumed to be a FIFO */
    while (1)
    {
        ctxt_newcontext();
        if (ctxt_setinput(fifo) != 0)
            sql_exit(1);
        sql_file();
        ctxt_endcontext();
    }
}

/* Implement sqlreload command -- load one table */
void sql_reload(char *tname)
{
    char buffer[SQL_CMDBUFSIZE];
    char skipbuff[32];
    long nskip = ctxt_getskipsize();

    if (nskip > 0)
        esnprintf(skipbuff, sizeof(skipbuff), "SKIP %ld", nskip);
    else
        skipbuff[0] = '\0';

    esnprintf(buffer, sizeof(buffer), "LOAD FROM \"%s\" %s INSERT INTO %s",
              ctxt_infile(), skipbuff, tname);
    do_command(buffer);
}

/* Convert string to file name */
/* Skips leading blanks; strips surrounding quotes and trailing blanks */
static char *filename(char *s)
{
    char           *t;
    char            c = '\0';

    s = t = skipblanks(s);
    if (*s == '\'' || *s == '"')
    {
        c = *s;
        t = ++s;
    }
    /* Allow spaces in quoted file names */
    while (*t != '\0' && *t != c && (c != '\0' || !isspace((unsigned char)*t)))
        t++;
    *t = '\0';
    if (c == '\0' && *(t - 1) == ';')
        *(t - 1) = '\0';
    return(s);
}

/* Execute internal command: CLOCK */
static void do_clock(char *s)
{
    char           *t;
    static Clock usr_clock; /* Not very neat, but very convenient */

    s = skipblanks(s);
    t = skipword(s);
    *t-- = '\0';
    if (*t == ';')
        *t = '\0';

    if (cistrcmp("start", s) == 0)
    {
        clk_start(&usr_clock);
    }
    else if (cistrcmp("stop", s) == 0)
    {
        char buffer[20];
        clk_stop(&usr_clock);
        (void)clk_elapsed_us(&usr_clock, buffer, sizeof(buffer));
        fprintf(ctxt_output(), "Time: %s\n", buffer);
    }
    else
    {
        cmd_warning(E_INVCLOCK, s);
    }
}

/* Execute internal command: INPUT */
static void do_input(char *s)
{
    char           *fn;

    fn = filename(s);
    if (*fn == '\0')
        fprintf(ctxt_output(), "%s\n", ctxt_infile());
    else
        sql_filename(fn);
}

/* Execute internal command: ERROR */
static void do_error(char *s)
{
    char           *fn;

    fn = filename(s);
    if (*fn == '\0')
        fprintf(ctxt_output(), "%s\n", ctxt_errfile());
    else
        (void)ctxt_seterror(fn);
}

/* Execute internal command: OUTPUT */
static void do_output(char *s)
{
    char           *fn;

    fn = filename(s);
    if (*fn == '\0')
        fprintf(ctxt_output(), "%s\n", ctxt_outfile());
    else
        (void)ctxt_setoutput(fn);
}

/* Process a set of stack operations, such as push toggle or push on */
static void do_setstack(char *s, void (*setter)(char const *))
{
    int counter = 0;
    char c = *s;
    do
    {
        char *e;
        s = skipblanks(s);
        e = skipalphanum(s);
        c = *e;
        *e = '\0';
        if (*s == '\0' && counter > 0)
            break;
        counter++;
        (*setter)(s);
        s = e;
        if (c != '\0')
            s++;
    } while (c != '\0');
}

/* Execute internal command: HEADINGS */
static void do_heading(char *s)
{
    do_setstack(s, ctxt_setheading);
}

/* Execute internal command: HEXMODE */
static void do_hexmode(char *s)
{
    do_setstack(s, ctxt_sethexmode);
}

/* Execute internal command: HELP */
static void do_help(char *s)
{
    fprintf(ctxt_output(), "%s\n", sqlcmd_help);
}

/* Execute internal command: TRANS */
static void do_transize(char *s)
{
    s = skipblanks(s);
    *skipalphanum(s) = '\0';
    ctxt_settransize(s);
}

/* Execute internal command: TYPES */
static void do_types(char *s)
{
    do_setstack(s, ctxt_settypes);
}

/* Execute internal command: VERBOSE */
static void do_verbose(char *s)
{
    do_setstack(s, ctxt_setverbose);
}

/* Execute internal command: CONTEXT */
static void do_context(char *s)
{
    s = skipblanks(s);
    if (*s != '\0' && *s != ';')
        error_746("Syntax error in CONTEXT command");
    else
        ctxt_print(ctxt_output());
}

/* Execute internal command: CONTINUE */
static void do_continue(char *s)
{
    do_setstack(s, ctxt_setcontinue);
}

/* Isolate format string: DATE, DATETIME, INTERVAL */
static char *isolate_format(char *s)
{
    char            q = '\0';
    char           *t;

    if (*s == '\'' || *s == '"')
    {
        t = s++;
        q = *t++;
        while (*t && *t != q)
            t++;
        *t = '\0';
    }
    t = s + strlen(s) - 1;
    if (q == '\0' && *t == ';')
        *t = '\0';
    while (t > s && isspace((unsigned char)*t))
        *t-- = '\0';
    return(s);
}

/* Execute internal command: DATE */
static void do_date(char *s)
{
    s = skipblanks(s);
    if (*s != '\0' && *s != ';')
    {
        char buffer[60];
        char *fmt = isolate_format(s);
        int rc;
        /**
        ** This test is not 100% reliable under GCC 2.8.1 on Solaris 2.6.
        ** It is probably more due to GCC than this code (JL 1998-06-29).
        ** For example: when coded in main(), this code fails with an
        ** invalid date, but when coded in a subroutine, it succeeds.
        */
        if ((rc = rfmtdate(0L, fmt, buffer)) != 0)
            error_746("Invalid format in DATE command");
        else
            ctxt_setdate(fmt);
    }
    else
        fprintf(ctxt_output(), "Date: %s\n", ctxt_getdate());
}

/* Execute internal command: EOR */
static void do_eor(char *s)
{
    s = skipblanks(s);
    ctxt_seteor(s);
}

/* Execute internal command: BLOBDIR */
static void do_blobdir(char *s)
{
    s = filename(s);
    if (*s != '\0')
        ctxt_setblobdir(s);
    else
        fprintf(ctxt_output(), "Blob Directory: %s\n", ctxt_getblobdir());
}

/* Execute internal command: DBNAMES */
static void do_dbnames(char *s)
{
    s = skipblanks(s);
    if (*s != '\0' && *s != ';')
        error_746("Syntax error in DBNAMES command");
    else
        sql_dbnames();
}

/* Execute internal command: IBASE */
static void do_ibase(char *s)
{
    s = skipblanks(s);
    *skipalphanum(s) = '\0';
    ctxt_setibase(s);
}

/* Execute internal command: QLIMIT */
static void do_qlimit(char *s)
{
    s = skipblanks(s);
    ctxt_setqlimit(s);
}

/* Execute internal command: DELIM */
static void do_delim(char *s)
{
    s = skipblanks(s);
    ctxt_setdelim(s);
}

/* Echo string to given output file */
static void echo_message(char *s, FILE *fp)
{
    char           *t;
    char            c;
    const char     *pad = "";

    s = skipblanks(s);
    while ((c = *s) != '\0')
    {
        if (c == '\'' || c == '"')
        {
            s++;
            t = strchr(s, c);
            if (t != NIL(char *))
                *t++ = '\0';
        }
        else
        {
            t = skipword(s);
            if (*t != '\0')
                *t++ = '\0';
        }
        if (*t == '\0' && *(t - 1) == ';')  /* Zap trailing semicolon */
            *(t - 1) = '\0';
        fprintf(fp, "%s%s", pad, s);
        s = t;
        pad = " ";
        s = skipblanks(s);
    }
    fputc('\n', fp);
    fflush(fp);
}

/* Execute internal command: ECHO */
static void do_echo(char *s)
{
    echo_message(s, ctxt_output());
}

/* Execute internal command: ERRMSG */
static void do_errmsg(char *s)
{
    echo_message(s, ctxt_error());
}

static void exec_sleep(double d)
{
#if defined(HAVE_CLOCK_NANOSLEEP)
    struct timespec req;
    req.tv_sec = d;
    req.tv_nsec = (d - req.tv_sec) * 1000000000.0 + 0.5;
    clock_nanosleep(CLOCK_REALTIME, 0, &req, 0);
#elif defined(HAVE_NANOSLEEP)
    struct timespec req;
    req.tv_sec = d;
    req.tv_nsec = (d - req.tv_sec) * 1000000000.0 + 0.5;
    nanosleep(&req, 0);
#elif defined(HAVE_USLEEP)
    if (d >= 1000.0)
    {
        int t = d;
        sleep(t);
    }
    else
    {
        useconds_t req = (d * 1000000 + 0.5);
        usleep(req);
    }
#elif defined(HAVE_SLEEP)
    int t = d;
    sleep(t);
#else
    error_746("SLEEP command not available");
#endif /* HAVE_*SLEEP */
}

/* Execute internal command: SLEEP */
static void do_sleep(char *s)
{
    double d = 0.0;
    if (sscanf(s, "%lf", &d) != 1 || d < 0.0)
        d = 0.0;
    if (d > SECS_IN_ONE_YEAR)
    {
        cmd_warning(E_INVSLEEP, s);
        d = SECS_IN_ONE_YEAR;
    }
    exec_sleep(d);
}

/* Execute internal command: ESCAPE */
static void do_escape(char *s)
{
    s = skipblanks(s);
    ctxt_setescape(s);
}

/* Execute internal command: VERSION */
static void do_version(char *s)
{
    s = skipblanks(s);
    if (*s != '\0' && *s != ';')
        error_746("Syntax error in VERSION command");
    else
        sql_version(ctxt_output());
}

/* Execute internal command: SQLSTART */
static void do_sqlstart(char *s)
{
    sqlstart();
}

/* Execute internal command: SQLSTOP */
static void do_sqlexit(char *s)
{
    sqlexit();
}

/* Execute internal command: EXIT/BYE/QUIT */
static void do_exit(char *s)
{
    long            c1;

    c1 = 0;
    sscanf(s, "%ld", &c1);
    sql_exit(c1);
    /* NOTREACHED */
}

/* Execute internal command: INVALID */
static void do_invalid(char *s)
{
    s = skipblanks(s);
    cmd_warning(E_INVCMD, s);
}

/* Execute internal command: NOTALLOWED */
static void do_notallowed(char *s)
{
    s = skipblanks(s);
    cmd_warning(E_CMDNOTALLOWED, s);
}

/* Execute internal command: QUOTE */
static void do_quote(char *s)
{
    s = skipblanks(s);
    ctxt_setquote(s);
}

/* Execute internal command: SHELL */
static void do_shell(char *s)
{
#if defined(SQLREAD) && defined(SQLREAD_NOSHELLESCAPE)
    do_notallowed("shell escape");
#else
    /* Shell is apt to get stroppy if given a semi-colon after a newline */
    /* This can happen if the user typed 'shell echo hi\n;' (where the   */
    /* semi-colon got added because SQLCMD prompted for more data.       */
    int rc;
    char *t = s + strlen(s) - 1;
    while (t >= s && *t == ';')
        *t-- = '\0';
    if ((rc = system(s)) != 0)
    {
        char buffer[16];
        snprintf(buffer, sizeof(buffer), "0x%.4X", rc);
        cmd_warning(E_SYSTEMFAIL, buffer);
    }
#endif /* SQLREAD && SQLREAD_NOSHELLESCAPE */
}

/* Execute internal command: SILENCE */
static void do_silence(char *s)
{
    do_setstack(s, ctxt_setsilence);
}

/* Execute internal command: SINGLE */
static void do_single(char *s)
{
    do_setstack(s, ctxt_setsingle);
}

/* Execute internal command: FORMAT */
static void do_format(char *s)
{
    char *t;
    char c;
    s = skipblanks(s);
    t = skipalphanum(s);
    c = *t;
    *t = '\0';
    ctxt_setformat(s);
    if (ctxt_getformat() == OP_XML)
    {
        if (c != '\0')
        {
            *t = c;
            s = skipblanks(t);
            xml_arghandler(s);
        }
    }
}

/* Execute internal command: HISTORY */
static void do_history(char *s)
{
    do_setstack(s, ctxt_sethistory);
}

/* Execute internal command: TIME */
static void do_time(char *s)
{
    s = skipblanks(s);
    if (*s != '\0' && *s != ';')
        error_746("Syntax error in TIME command");
    else
        pr_time(ctxt_output());
}

/* Execute internal command: BENCHMARK */
static void do_benchmark(char *s)
{
    do_setstack(s, ctxt_setbmark);
}

/* Execute internal command: TRACE */
static void do_trace(char *s)
{
    do_setstack(s, ctxt_settrace);
}

static void set_load_context(LoadInfo *load)
{
    const char *str;
    char qstring[20];
    if (load->delim[0])
        ctxt_setdelim(quotify(load->delim, qstring, sizeof(qstring)));
    if (load->quote[0])
        ctxt_setquote(quotify(load->quote, qstring, sizeof(qstring)));
    if (load->escape[0])
        ctxt_setescape(quotify(load->escape, qstring, sizeof(qstring)));
    if (load->eor[0])
        ctxt_seteor(quotify(load->eor, qstring, sizeof(qstring)));
    if (load->format[0])
        ctxt_setformat(load->format);
    if (load->xmltag[0])
        ctxt_setxmlrecordtag(load->xmltag);
    /* Always set SKIP - to zero if not set by user */
    if (load->skip[0])
        str = load->skip;
    else
        str = "0";
    ctxt_setskipsize(str);
}

#ifndef SQLREAD
/* Load data from file/pipe using insert statement */
static void load_file(LoadInfo *load)
{
    int rc;
    ctxt_newcontext();
    rc = ctxt_setloadinput(load->file, load->pipe_or_file, load->mode);
    if (rc == 0)
    {
        set_load_context(load);
        reload(load->stmt);
    }
    ctxt_endcontext();
}
#endif /* SQLREAD */

/* Unload data to file/pipe using SELECT statement */
static void unload_file(LoadInfo *load)
{
    int rc;
    ctxt_newcontext();
    rc = ctxt_setunloadoutput(load->file, load->pipe_or_file, load->mode);
    if (rc == 0)
    {
        if (ctxt_getformat() == OP_SELECT)
            ctxt_setformat(op_unload);
        ctxt_settrace(op_off);
        set_load_context(load);
        /* This is where to alter blob directory and blob location */
        sql_command(load->stmt);
    }
    ctxt_endcontext();
}

/* Parse and interpret LOAD, UNLOAD or RELOAD statement */
/* RELOAD implies transaction size and similar handling */
static void do_load_op(char *buffer)
{
    LoadInfo load;

    switch (parse_loadstmt(buffer, &load))
    {
    case STMT_RELOAD:
#ifdef SQLREAD
        error_746("The RELOAD statement is not permitted");
#else
        if (ctxt_getformat() == OP_XML)
            error_746("XML-format input is not handled in LOAD or RELOAD");
        else
        {
            int old_pflag = pflag;
            pflag = SQLRELOAD;
            load_file(&load);
            pflag = old_pflag;
        }
#endif /* SQLREAD */
        break;

    case STMT_LOAD:
#ifdef SQLREAD
        error_746("The LOAD statement is not permitted");
#else
        if (ctxt_getformat() == OP_XML)
            error_746("XML-format input is not handled in LOAD or RELOAD");
        else
            load_file(&load);
#endif /* SQLREAD */
        break;

    case STMT_UNLOAD:
        unload_file(&load);
        break;

    case STMT_UNLOADERR:
        error_746("Syntax error in UNLOAD statement");
        break;
    case STMT_LOADERR:
        error_746("Syntax error in LOAD statement");
        break;
    case STMT_RELOADERR:
        error_746("Syntax error in RELOAD statement");
        break;

    default:
        error_746("Syntax/internal error; unrecognized statement - LOAD/RELOAD/UNLOAD");
        break;
    }
}

/* Implement sqlunload command -- unload one table */
void sql_unload(char *tname, char *order_by)
{
    char            buffer[SQL_CMDBUFSIZE];

    esnprintf(buffer, sizeof(buffer), "UNLOAD TO \"%s\" SELECT * FROM %s", ctxt_outfile(), tname);
    if (order_by != 0)
    {
        size_t len = strlen(buffer);
        nvstrcpy(&buffer[len], sizeof(buffer)-len, 2, " ORDER BY ", order_by);
    }
    do_command(buffer);
}

/* Rerun history command(s) */
static void do_rerun(char *s)
{
    long            c1;
    long            c2;
    FILE           *fp;

    if (ctxt_gethistory() != OP_ON)
    {
        cmd_warning(E_HISTORYOFF, "");
        return;
    }

    s = skipblanks(s);
    c1 = c2 = 0;
    if (sscanf(s, "%ld%ld", &c1, &c2) != 2)
        c2 = c1;
    fp = tmpfile();
    hist_output(fp, c1, c2, H_COMMAND);
    fseek(fp, 0L, SEEK_SET);
    if ((c1 = hist_input(fp)) > 0)
        cmd_set_promptnum(c1);
    fseek(fp, 0L, SEEK_SET);
    ctxt_newcontext();
    ctxt_newinput(fp, "<<temp>>");
    ctxt_sethistory(op_off);
    sql_file();
    ctxt_endcontext();  /* Closes fp! */
}

/* Erase (delete) history command(s) */
/* Delete is pre-empted by SQL, of course */
static void do_erase(char *s)
{
    long            c1;
    long            c2;

    if (ctxt_gethistory() != OP_ON)
    {
        cmd_warning(E_HISTORYOFF, "");
        return;
    }

    s = skipblanks(s);
    c1 = c2 = 0;
    if (sscanf(s, "%ld%ld", &c1, &c2) != 2)
        c2 = c1;
    hist_erase(c1, c2);
}

#if !(defined(SQLREAD) && defined(SQLREAD_NOSHELLESCAPE))

/* Create and open a named temporary file */
/* NB: copies opened name into fname variable! */
static FILE *fopen_namedtmpfile(char *fname, size_t bufsiz)
{
    FILE *fp = 0;
    const char *tmpdir = sql_dbtemp();  /* ixblob.h */
    size_t len;

    if (tmpdir == 0)
        err_syserr("Failed to determine temporary directory\n");

    len = strlen(tmpdir) + sizeof(TMPFILE_TEMPLATE) + sizeof("/");
    if (bufsiz < len)
        err_error("Configuration error: editor file name buffer too short\n"
                  "(or $DBTEMP or $TMPDIR too long!)\n");
    esnprintf(fname, bufsiz, "%s/%s", tmpdir, TMPFILE_TEMPLATE);
    fp = fmkstemp(fname);
    return fp;
}

#endif /* !(SQLREAD && SQLREAD_NOSHELLESCAPE) */

#if defined(SQLREAD) && defined(SQLREAD_NOSHELLESCAPE)

/* Edit history command(s) - no shell escape version */
static void do_edit(char *s)
{
    do_notallowed("command history editing");
}

#else

/* Edit history command(s) */
static void do_edit(char *s)
{
    int             rc;
    FILE           *fp;
    long            c1;
    long            c2;
    char            tmpfname[BUFSIZ];
    char            sys[BUFSIZ];
    const char     *editor;

    if (ctxt_gethistory() != OP_ON)
    {
        cmd_warning(E_HISTORYOFF, "");
        return;
    }

    s = skipblanks(s);
    c1 = c2 = 0;
    if (sscanf(s, "%ld%ld", &c1, &c2) != 2)
        c2 = c1;

    if ((fp = fopen_namedtmpfile(tmpfname, sizeof(tmpfname))) == 0)
    {
        cmd_warning(E_FAILCREATETMPFILE, "");
        return;
    }

    hist_output(fp, c1, c2, H_COMMAND);
    fclose(fp);

    if ((editor = getenv("DBEDIT")) == NIL(char *) &&
        (editor = getenv("VISUAL")) == NIL(char *) &&
        (editor = getenv("EDITOR")) == NIL(char *))
        editor = DEF_EDITOR;
    esnprintf(sys, sizeof(sys), "%s %s", editor, tmpfname);
    if ((rc = system(sys)) != 0)
    {
        char buffer[16];
        snprintf(buffer, sizeof(buffer), "0x%.4X", rc);
        cmd_warning(E_EDITORFAIL, buffer);
    }

    fp = fopen(tmpfname, "r");
    unlink(tmpfname);
    if (fp == 0)
    {
        cmd_warning(E_FAILREOPENTMPFILE, tmpfname);
    }
    else
    {
        /* Copy file to history log */
        if ((c1 = hist_input(fp)) > 0)
            cmd_set_promptnum(c1);
        fseek(fp, 0L, SEEK_SET);
        ctxt_newcontext();
        ctxt_newinput(fp, "<<temp>>");
        ctxt_sethistory(op_off);
        sql_file();
        ctxt_endcontext();
    }
}
#endif /* SQLREAD && SQLREAD_NOSHELLESCAPE */

/* List history command(s) */
static void do_list(char *s)
{
    long            c1;
    long            c2;

    if (ctxt_gethistory() != OP_ON)
    {
        cmd_warning(E_HISTORYOFF, "");
        return;
    }

    s = skipblanks(s);
    c1 = c2 = 0;
    if (sscanf(s, "%ld%ld", &c1, &c2) != 2)
        c2 = c1;
    hist_output(ctxt_output(), c1, c2, H_NUMBERS);
    /* Dubious error handling? */
    cmd_set_promptnum(hist_getcmdnum());
}

/* Adjust size of history file */
static void do_adjust(char *s)
{
    long            c1;

    if (ctxt_gethistory() != OP_ON)
    {
        cmd_warning(E_HISTORYOFF, "");
        return;
    }

    s = skipblanks(s);
    c1 = 0;
    sscanf(s, "%ld", &c1);
    if (c1 == 0)
        fprintf(ctxt_output(), "History size: %lu\n", (Ulong)hist_getsize());
    else
        hist_setsize(c1);
}

/* Initialize the history file */
void hist_init(void)
{
    long            cmdnum;
    Error           errnum;

    if ((cmdnum = hist_open(H_MODIFY)) >= 0)
    {
        cmd_set_promptnum(cmdnum);
#ifdef HAVE_READLINE_READLINE_H
        {
            int     i;
            for (i = 1; i <= cmdnum; i++)
            {
                char *cmd = hist_read(i);
                cmd_add_to_readline_history(cmd);
                free(cmd);
            }
        }
#endif
    }
    else
    {
        switch (cmdnum)
        {
        case H_OLDMAGIC:
            errnum = E_OLDMAGIC;
            break;
        case H_BADMAGIC:
            errnum = E_BADMAGIC;
            break;
        case H_BADFORMAT:
            errnum = E_BADFORMAT;
            break;
        case H_CANTOPEN:
            errnum = E_CANTOPEN;
            break;
        case H_LOCKERR:
            errnum = E_LOCKERR;
            break;
        default:
            errnum = E_INTERNAL;
            break;
        }
        pflag = SQLUNLOAD;      /* Does not recover */
        cmd_error(errnum, hist_file());
        /* NOTREACHED */
    }
}

/* Execute internal command: XML */
/* Probably simpler, and more reliable, and more flexible, to do it via the grammar. */
static void do_xml(char *s)
{
    char *t;
    char  empty[] = ""; /*=C++=*/
    char *v = empty;

    s = skipblanks(s);
    t = skipword(s);
    if (*t == '\0')
    {
        if (t > s && *(t-1) == ';')
            *--t = '\0';
    }
    else if (*t == ';')
    {
        *t = '\0';
    }
    else if (*t != '\0')
    {
        *t++ = '\0';
        v = skipblanks(t);
        t = skipword(v);
        *t-- = '\0';
        if (*t == ';')
            *t = '\0';
    }

    if (cistrcmp("record", s) == 0)
    {
        if (*v == '\0')
            fprintf(ctxt_output(), "XML Record Tag: %s\n", ctxt_getxmlrecordtag());
        else
            ctxt_setxmlrecordtag(v);
    }
    else if (cistrcmp("recset", s) == 0)
    {
        if (*v == '\0')
            fprintf(ctxt_output(), "XML Record Set Tag: %s\n", ctxt_getxmlrecsettag());
        else
            ctxt_setxmlrecsettag(v);
    }
    else if (cistrcmp("header", s) == 0)
    {
        if (*v == '\0')
            fprintf(ctxt_output(), "XML Header Tag: %s\n", ctxt_getxmlheadertag());
        else
            ctxt_setxmlheadertag(v);
    }
    else
    {
        fprintf(ctxt_output(), "XML Record Set Tag: %s\n", ctxt_getxmlrecsettag());
        fprintf(ctxt_output(), "XML Record Tag:     %s\n", ctxt_getxmlrecordtag());
        fprintf(ctxt_output(), "XML Header Tag:     %s\n", ctxt_getxmlheadertag());
    }
}
