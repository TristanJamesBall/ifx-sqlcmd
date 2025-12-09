/*
@(#)File:           $RCSfile: sqlcmd.c,v $
@(#)Version:        $Revision: 2016.2 $
@(#)Last changed:   $Date: 2016/07/29 05:40:17 $
@(#)Purpose:        Main Program for SQLCMD
@(#)Author:         J Leffler
@(#)Copyright:      (C) JLSS 1992-93,1997-2011,2013,2015-16
@(#)Product:        SQLCMD Version 90.02 (2016-07-28)
*/

/*TABSTOP=4*/

#define MAIN_PROGRAM
#define EXTERN          /* Defines variables in sqlcmd.h */
#define USE_JLSS_GETOPT
#define USE_JLSS_GETSUBOPT

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include "posixver.h"

#include <assert.h>
#include <setjmp.h>
#include <stdio.h>
#include <string.h>

#include "context.h"
#include "connect.h"    /* For complete list of headers */
#include "debug.h"
#include "emalloc.h"
#include "esqlc.h"
#include "describe.h"
#include "getopt.h"
#include "history.h"
#include "internal.h"
#include "output.h"
#include "sqlcmd.h"
#include "sqlerr.h"
#include "jlsstools.h"
#include "stderr.h"
#include "jlss.h"
#include "chktty.h"

/* -- Declarations */

int pflag = 0;      /* Program operating mode: SQLCMD, SQLRELOAD, etc */

/*
** SQLCMD currently uses 19 lower-case and 24 upper-case option letters
** (not K, W).  Options reserved for future integration of SQLUPLOAD
** into SQLCMD:
**      -l logfile
**      -r rejfile
**      -k keycolumns - unique key columns
**      -K keycolumns - non-unique key columns
**      -? owner (of table; maybe included in -t table; SQLUPLOAD used -o)
**      -m algorithm (select, insert, update, heuristic - SQLUPLOAD used -S, -I, -U, -H)
**      -? lockmode (shared or exclusive - SQLUPLOAD used -x, -s)
**      -? stop after errors - SQLUPLOAD used -e
**      -? column list in input file... - SQLUPLOAD used -c
** NB: may opt to use:
**      -k columnlist - key.column used to denote (old) key values, as in UPLOAD stmt.
**      -K unique|nonunique - to indicate whether key is unique or not (default unique)
**      -q quiet mode
**      -j (reJect) for stop after errors.
*/

static const char optlist[] = "a:bcd:e:f:ghi:o:p:st:u:vxyz:A:BCD:E:F:G:HIJL:M:N:O:Q:RS:TUVX:YZ:";

static const char usestr[] =
"[-bcghsvxyY] [-d dbase] [-f file] [-e 'SQL stmt'] ['SQL stmt' ...]\n"
"Other options: [-CJRU][-BHITV][-D delim][-E escape][-Q quote][-F format]\n"
"               [-A date][-i file][-o file][-L qlimit][-N number][-M FIFO]\n"
"               [-Z debug][-G eor][-t table][-p password][-u username]\n"
"               [-a ibase][-X record=rectag,recset=settag,header=hdrtag]\n"
"               [-O orderby][-S skip][-z number]\n"
"NB: -h gives more help!";

static const char fullhelp[] =
"\nOption summary:\n"
"   -a ibase      - input base (default 0; set to 10 for fields with leading zeroes)\n"
"   -b            - support hex escapes (DB-Access -X)\n"
"   -c            - continue after errors\n"
"   -d dbase      - select database\n"
"   -e 'SQL stmt' - execute SQL command\n"
"   -f file       - input file for SQLCMD\n"
"   -g            - debugging mode (single-step commands)\n"
"   -h            - print this message\n"
"   -i file       - input file for SQLRELOAD\n"
/*  -j            - unused */
/*  -k            - reserved (UPLOAD key) */
/*  -l            - reserved (UPLOAD log) */
/*  -m            - reserved (UPLOAD mode) */
/*  -n            - no response to PAM challenge/response authentication (bust) */
"   -o file       - output file for SQLUNLOAD\n"
"   -p password   - password for connection (BEWARE: security implications!)\n"
/*  -q            - unused */
/*  -r            - reserved (UPLOAD rejects) */
"   -s            - silent mode\n"
"   -t table      - name of table (for SQLRELOAD or SQLUNLOAD)\n"
"   -u username   - username for connection (BEWARE: security implications!)\n"
"   -v            - verbose mode\n"
"   -x            - trace mode\n"
"   -y            - enable history\n"
"   -z number     - set debugging level for syntax and lexical analyzers\n"
"   -A date       - set date format (eg dd-mm-yyyy for $DBDATE=dmy4-)\n"
"   -B            - operate in benchmark mode (implies -x and times SQL)\n"
"   -C            - operate as SQLCMD\n"
"   -D delim      - set field delimiter (default $DBDELIMITER or pipe)\n"
"   -E escape     - set escape character (default $DBESCAPE or backslash)\n"
"   -F format     - set output format (default SELECT; alternatives include:\n"
"                   UNLOAD, FIXED, FIXSEP, FIXDEL, CSV, XML, HTML, BLOCK, MARKDOWN )\n"
"                   WARNING: Markdown mode is ASCII only and strips unprintable characters\n"
"                            Newlines are converted to <br> while '\\'s and '|'s\n"
"                            are escaped according to GH Markdown rules\n"
"   -G eor        - set EOR (end of record) character string (default $DBNEWLINE or newline)\n"
"   -H            - include column headings in output\n"
"   -I            - interactive mode (after executing command line)\n"
"   -J            - simulate isql/dbaccess; accept [database|-] [script|-]\n"
/*  -K            - unused */
"   -L qlimit     - limit on number of rows returned by query\n"
"   -M FIFO       - monitor FIFO for input commands\n"
"   -N number     - size of transaction under RELOAD\n"
"                   (default 1024; 0 means no sub-transactions)\n"
"   -O col1,...   - order by these columns (SQLUNLOAD)\n"
/*  -P            - operate as SQLUPLOAD (not implemented yet) */
"   -Q quote      - set quote character (default $DBQUOTE or double quote)\n"
"   -R            - operate as SQLRELOAD\n"
"   -S skip       - initial rows to skip (SQLRELOAD)\n"
"   -T            - include column types in output\n"
"   -U            - operate as SQLUNLOAD\n"
"   -V            - print version and exit\n"
/*  -W blobdir    - default blob directory */
"   -X xmloptions - configure the XML output\n"
"      recset='recsettag',record='recordtag',header='headertag'\n"
"   -Y            - do not enable history, even in interactive mode\n"
"   -Z debug      - set debugging level (SQLCMD must be compiled with debugging enabled)\n"
;

static const char mutexopts[] = "(-C, -J, -R, -U)";

#ifdef HAVE_SIGLONGJMP
static sigjmp_buf jb;
#else
static jmp_buf    jb;
#endif /* HAVE_SIGLONGJMP */

static void arg_handler(int argc, char **argv);
static void sql_interactive(int Yflag);
static int  sql_progmode(const char *prog);

#ifndef lint
/* Prevent over-aggressive optimizers from eliminating ID string */
extern const char jlss_id_sqlcmd_c[];
const char jlss_id_sqlcmd_c[] = "@(#)$Id: sqlcmd.c,v 2016.2 2016/07/29 05:40:17 jleffler Exp $";
#endif /* lint */

int main(int argc, char **argv)
{
    err_setarg0(argv[0]);
    sql_signals();
    pflag = sql_progmode(err_getarg0());
    ctxt_init();
    putenv("IFX_DEFERRED_PREPARE=0");   /* Disable deferred prepare */

    /* Error return point - until sql_interactive() at work */
#ifdef HAVE_SIGLONGJMP
    if (sigsetjmp(jb, 1) != 0)
#else
    if (setjmp(jb) != 0)
#endif /* HAVE_SIGLONGJMP */
        sql_exit(1);

    arg_handler(argc, argv);

    sql_exit(0);
    return(0);
}

int xml_arghandler(const char *argval)
{
    static char * const xmlopts[] =
    {
        "header",
        "record",
        "recset",
        0
    };
    char *argdup = STRDUP(argval);
    char *arg = argdup;
    char *str;
    int   rc = 0;

    for (str = arg; *str != '\0'; arg = str)
    {
        char *val;
        /* printf("str: %s\n", str);    // Beware: C99! */
        int subopt = GETSUBOPT(&str, xmlopts, &val);
        /* printf("sub: %d, str = %s, val = %s\n", subopt, str, val ? val : "<null>"); */
        if (val == (char *)0 || *val == '\0')
        {
            cmd_warning(E_MISSINGSUBOPT, xmlopts[subopt]);
            rc = -1;
        }
        switch (subopt)
        {
        case 0:
            ctxt_setxmlheadertag(val);
            break;
        case 1:
            ctxt_setxmlrecordtag(val);
            break;
        case 2:
            ctxt_setxmlrecsettag(val);
            break;
        default:
            cmd_warning(E_INVSUBOPT, argval);
            rc = -1;
            break;
        }
    }
    FREE(argdup);
    return(rc);
}

static char const SQL_SUFFIX[] = ".sql";
enum { SQL_SUFFIX_LEN = sizeof(SQL_SUFFIX) - 1 };

/* Add suffix .sql to file name if not already present */
static char *add_sql_suffix(const char *base)
{
    size_t len = strlen(base);
    char *file;
    if (len <= SQL_SUFFIX_LEN || strcmp(base + len - SQL_SUFFIX_LEN, SQL_SUFFIX) != 0)
    {
        file = MALLOC(len + SQL_SUFFIX_LEN + 1);
        strcpy(file, base);
        strcpy(file + len, SQL_SUFFIX);
    }
    else
        file = STRDUP(base);
    return(file);
}

/* Code uses a single character for quote, delim, escape; it allows up to 7 for EOR */
/* Even with excruciating escaping, 20 is way longer than a plausible expansion */
static void set_argstring(char *arg, char opt, const char *field, int maxlen, void (*setter)(const char *))
{
    char qstring[20];
    if (strlen(arg) >= sizeof(qstring))
        err_error("Input value for %s (-%c '%s') is far too long (max %d)\n", field, opt, arg, maxlen);
    (*setter)(quotify(arg, qstring, sizeof(qstring)));
}

/* Process command line arguments */
static void     arg_handler(int argc, char **argv)
{
    int             opt;
    int             uflag = pflag;
    int             cmd_flag = 0;
    int             dflag = 0;
    int             iflag = 0;
    int             jflag = 0;
    int             Yflag = 1;  /* Interactive history enabled by default */
    char           *tname = NIL(char *);
    char           *fifo = NIL(char *);
    char           *username = NIL(char *);
    char           *password = NIL(char *);
    char           *order_by = NIL(char *);
    char           *arg;

    /* Process option list */
    while ((opt = GETOPT(argc, argv, optlist)) != EOF)
    {
        switch (opt)
        {
        case 'a':               /* Set input base */
            ctxt_setibase(optarg);
            break;

        case 'b':
            ctxt_sethexmode(op_on);
            break;

        case 'c':               /* Continue on error */
            ctxt_setcontinue(op_on);
            break;

        case 'd':               /* Select database */
            sql_dbconnect(optarg, username, password);
            dflag = 1;
            break;

        case 'e':               /* Execute SQL command */
            if (pflag != SQLCMD)
                cmd_error(E_INVOPTION, "e");
            pflag = SQLEARG;
            do_command(optarg);
            pflag = uflag;
            jflag = 1;
            break;

        case 'f':               /* Execute commands from file */
            if (pflag != SQLCMD)
                cmd_error(E_INVOPTION, "f");
            pflag = SQLFARG;
            sql_filename(optarg);
            pflag = uflag;
            jflag = 1;
            break;

        case 'g':               /* Debug (single-step) */
            ctxt_setsingle(op_on);
            break;

        case 'h':               /* Report usage (help) */
            sql_version(ctxt_output());
            err_logmsg(ctxt_output(), ERR_ERR|ERR_NOARG0, EXIT_SUCCESS,
                       "Usage: %s %s\n%s", err_getarg0(), usestr, fullhelp);
            break;

        case 'i':               /* Input from file */
#ifdef SQLREAD
            cmd_error(E_OPTNOTALLOWED, "i");
#else
            if (pflag != SQLRELOAD)
                cmd_error(E_INVOPTION, "i");
            if (ctxt_setinput(optarg) != 0)
                sql_exit(1);
#endif /* SQLREAD */
            break;

        case 'o':               /* Output to file */
            if (pflag != SQLUNLOAD)
                cmd_error(E_INVOPTION, "o");
            if (ctxt_setoutput(optarg) != 0)
                sql_exit(1);
            break;

        case 'p':               /* Password - dubious security */
            password = STRDUP(optarg);
            /*
            ** Overwrite password to reduce the chance of it being seen
            ** via ps, but it does not do anything useful on Solaris 7.
            */
            memset(optarg, 'X', strlen(optarg));
            if (dflag)
                err_logmsg(ctxt_output(), ERR_REM, EXIT_SUCCESS,
                           "Password set after connection completed for -d option");
            break;

        case 's':               /* Silent mode */
            ctxt_setsilence(op_on);
            break;

        case 't':               /* Table to load/unload */
            if (pflag == SQLCMD)
                cmd_error(E_INVOPTION, "t");
            if (tname != NIL(char *))
                cmd_error(E_DUPTABLE, optarg);
            tname = optarg;
            break;

        case 'u':               /* Username */
            username = optarg;
            if (dflag)
                err_logmsg(ctxt_output(), ERR_REM, EXIT_SUCCESS,
                           "User name set after connection completed for -d option");
            break;

        case 'v':               /* Verbose output (row counts, warnings) */
            ctxt_setverbose(op_on);
            break;

        case 'x':               /* Trace commands */
            ctxt_settrace(op_on);
            break;

        case 'y':               /* Set history on */
            /* JL 2001-09-03: Renamed from -h */
            if (pflag != SQLCMD)
                cmd_error(E_INVOPTION, "y");
            hist_init();
            ctxt_sethistory(op_on);
            break;

        case 'z':               /* Set grammar/lexical analyzer debugging */
            set_yydebug(atoi(optarg));
            break;

        case 'A':               /* Date format */
            ctxt_setdate(optarg);
            break;

        case 'B':               /* Benchmark mode */
            ctxt_setbmark(op_on);
            break;

        case 'C':               /* SQLCMD */
            if (cmd_flag != 0)
                cmd_error(E_MUTEXOPTS, mutexopts);
            cmd_flag = pflag = SQLCMD;
            break;

        case 'D':               /* Delimiter character */
            set_argstring(optarg, opt, "delimiter", 1, ctxt_setdelim);
            break;

        case 'E':               /* Escape character */
            set_argstring(optarg, opt, "escape", 1, ctxt_setescape);
            break;

        case 'F':               /* Output format */
            ctxt_setformat(optarg);
            break;

        case 'G':               /* End-of-record string */
            set_argstring(optarg, opt, "end-of-record", 7, ctxt_seteor);
            break;

        case 'H':               /* Print column headings */
            ctxt_setheading(op_on);
            break;

        case 'I':               /* Go interactive after command processing */
            if (pflag != SQLCMD)
                cmd_error(E_INVOPTION, "I");
            iflag = 1;
            break;

        case 'J':               /* DBACCESS/ISQL mode */
            if (cmd_flag != 0)
                cmd_error(E_MUTEXOPTS, mutexopts);
            cmd_flag = pflag = DBACCESS;
            break;

        case 'L':               /* Set maximum rows printed by SELECT */
            if (pflag != SQLCMD)
                cmd_error(E_INVOPTION, "L");
            ctxt_setqlimit(optarg);
            break;

        case 'M':
            if (pflag != SQLCMD)
                cmd_error(E_INVOPTION, "M");
            fifo = optarg;
            break;

        case 'N':               /* Transaction size */
            ctxt_settransize(optarg);
            break;

        case 'O':               /* ORDER BY clause */
            if (pflag != SQLUNLOAD)
                cmd_error(E_INVOPTION, "O");
            order_by = optarg;
            break;

        case 'Q':               /* Quote character */
            set_argstring(optarg, opt, "quote", 1, ctxt_setquote);
            break;

        case 'R':               /* SQLRELOAD */
#ifdef SQLREAD
            cmd_error(E_OPTNOTALLOWED, "R");
#else
            if (cmd_flag != 0)
                cmd_error(E_MUTEXOPTS, mutexopts);
            cmd_flag = pflag = SQLRELOAD;
#endif /* SQLREAD */
            break;

        case 'S':               /* Skip - for SQLRELOAD */
#ifdef SQLREAD
            cmd_error(E_OPTNOTALLOWED, "S");
#else
            ctxt_setskipsize(optarg);
#endif /* SQLREAD */
            break;

        case 'T':               /* Print column types */
            ctxt_settypes(op_on);
            break;

        case 'U':               /* SQLUNLOAD */
            if (cmd_flag != 0)
                cmd_error(E_MUTEXOPTS, mutexopts);
            cmd_flag = pflag = SQLUNLOAD;
            break;

        case 'V':               /* Print version */
            sql_version(ctxt_output());
            jflag = 1;
            break;

        case 'X':
            if (xml_arghandler(optarg) != 0)
                sql_exit(1);
            break;

        case 'Y':
            Yflag = 0;
            break;

        case 'Z':
            db_setdebug(atoi(optarg));
            break;

        default:
            err_usage(usestr);
            break;
        }
    }

    if (password != 0)
    {
        /* Clean password and user name - not to be used in DB-Access mode */
        memset(password, 'X', strlen(password));
        FREE(password);
        password = NIL(char *);
        username = NIL(char *);
    }

    if (iflag)
        jflag = 0;

    if (pflag == SQLUPLOAD)
    {
        error_746(SQL_UPLOAD " not implemented in " SQL_CMD " yet.  Sorry!");
    }
    else if (pflag == DBACCESS)
    {
        if (jflag)
            ;
        else if (optind >= argc - 2)
        {
            /* "dbaccess" - no options?  sqlcmd ditto! */
            /* "dbaccess {dbase|-}" */
            /* "dbaccess {dbase|-} {script|-}" */
            if (dflag)
                error_746("Cannot use -d option in this mode.");
            ctxt_setformat(op_block);
            if (optind < argc && strcmp(argv[optind], "-") != 0)
                sql_dbconnect(argv[optind], username, password);
            optind++;
            if (optind < argc && strcmp(argv[optind], "-") != 0)
            {
                char *file_sql = add_sql_suffix(argv[optind]);
                sql_filename(file_sql);
                free(file_sql);
            }
            else
            {
                /* Process standard input */
                sql_interactive(Yflag);
            }
        }
        else
        {
            cmd_error(E_STRAYARG, argv[optind + 2]);
        }
    }
    else if (pflag == SQLUNLOAD || pflag == SQLRELOAD)
    {
        /* Either SQLUNLOAD or SQLRELOAD */
        if (jflag)
            ;
        else if (dflag == 0)
            cmd_error(E_NODBASE, "");
        else if (tname == NIL(char *))
            cmd_error(E_NOTABLE, "");
        else if (optind != argc)
            cmd_error(E_STRAYARG, argv[optind]);
        else if (pflag == SQLRELOAD)
            sql_reload(tname);
        else
            sql_unload(tname, order_by);
    }
    else if (fifo != NIL(char *))
    {
        if (jflag != 0)
            cmd_error(E_INVFIFO, "");
        else if (optind != argc)
            cmd_error(E_STRAYARG, argv[optind]);
        pflag = SQLFIFO;
        sql_signals();
        sql_monitor(fifo);
    }
    else if (optind != argc)
    {
        /* Stray arguments */
        /* Those which contain blanks are treated as commands */
        /* Those which don't are treated as file names */
        while (optind < argc)
        {
            arg = argv[optind++];
            if (strchr(arg, ' ') != NIL(char *))
            {
                pflag = SQLEARG;
                do_command(arg);
                pflag = uflag;
            }
            else
            {
                pflag = SQLFARG;
                sql_filename(arg);
                pflag = uflag;
            }
        }
    }
    else if (jflag == 0)
    {
        /* Process standard input */
        sql_interactive(Yflag);
    }
}

static void sql_interactive(int Yflag)
{
    if (chk_tty(stdin))
    {
        ctxt_setcontinue(op_on);
        if (Yflag)
        {
            hist_init();
            ctxt_sethistory(op_on);
        }
    }
#ifdef HAVE_SIGLONGJMP
    if (sigsetjmp(jb, 1) != 0)
#else
    if (setjmp(jb) != 0)
#endif /* HAVE_SIGLONGJMP */
    {
        if (ctxt_getcontinue() != OP_ON)
            sql_exit(1);
        jb_release();           /* Release all the register memory */
        ctxt_recover();         /* Recover the base context */
    }
    sql_file();
}

void sql_longjmp(int status)
{
#ifdef HAVE_SIGLONGJMP
    siglongjmp(jb, status);
#else
    longjmp(jb, status);
#endif /* HAVE_SIGLONGJMP */
}

/* Determine which program was invoked */
static int sql_progmode(const char *prog)
{
    int             flag;

    if (strprefix(prog, SQL_UNLOAD))
        flag = SQLUNLOAD;
    else if (strprefix(prog, SQL_RELOAD))
        flag = SQLRELOAD;
    else if (strprefix(prog, SQL_DBACCESS))
        flag = DBACCESS;
    else if (strprefix(prog, SQL_ISQL))
        flag = DBACCESS;
#ifdef FUTURE_PERFECT
    else if (strprefix(prog, SQL_UPLOAD))
        flag = SQLUPLOAD;
#endif /* FUTURE_PERFECT */
    else
        flag = SQLCMD;
    return(flag);
}
