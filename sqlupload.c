/*
@(#)File:           $RCSfile: sqlupload.c,v $
@(#)Version:        $Revision: 2013.1 $
@(#)Last changed:   $Date: 2013/09/08 23:19:13 $
@(#)Purpose:        Main program for INSERT/UPDATE loader
@(#)Author:         J Leffler
@(#)Copyright:      (C) JLSS 1998-2000,2002-03,2005-06,2008,2013
@(#)Product:        SQLCMD Version 90.02 (2016-07-28)
*/

/*TABSTOP=4*/

/*
** Program to UPDATE or INSERT rows into a table from a load file
**
** This program has a very complex set of options, mainly because it does a
** very complex job.
**
** The options are:
**
**  -V              -- print version information and exit
**  -d dbase        -- database
**  -u username     -- username to use when connecting
**  -p password     -- password to use when connecting
**  -o owner        -- owner of table
**  -t table        -- table name
**  -k pkcols       -- primary key columns of table
**  -K cols         -- non-unique columns used to update table
**  -l logfile      -- log file for errors
**  -r rejects      -- reject records file
**  -n rows         -- rows per transaction
**  -e errors       -- errors before abort
**  -i skip         -- rows to skip before starting
**  -f input        -- input file
**  -q              -- quiet mode
**  -v              -- verbose mode
**  -s              -- lock table in shared mode
**  -x              -- lock table in exclusive mode
**  -H (default)    -- use heuristic to decide whether to UPDATE or INSERT
**  -I              -- do INSERT before UPDATE
**  -S              -- do SELECT to decide whether to UPDATE or INSERT
**  -U              -- do UPDATE before INSERT
**  -c cols         -- columns present in data file
**  -D delim        -- delimiter in data file
**  -E escape       -- escape in data file
**  -Z debug        -- set debug level
**
** Obviously, the -s and -x options are mutually exclusive.
** The -q and -v options are mutually exclusive.
** The -I, -S and -U options are mutually exclusive.
** The -k and -K options are mutually exclusive.
** The -d and -t options are mandatory.
** One of the -k or -K options is mandatory.
** To be effective, both the -u and the -p options must be specified.
** Having the password specified on the command line is not ideal for
** security reasons.
** The other options have defaults and can be omitted.
**
** The -V option stops the program without doing anything other than
** printing the version information.  It only works if there are no
** errors in the command line prior to it appearing.
**
** The -d option specifies the database and can include the server name if
** required.  The -t option specifies the table name.  If the table
** name needs to be qualified with the owner, then the owner name can
** be specified with the -o option.  The -o option is case-sensitive;
** the supplied value will be embedded in quotes and should match
** exactly the value in "informix".SysTables.Owner.  Probable future
** upgrade: allow owner.tablename after -t option.  The -u and -p
** options have to be specified together and allow the program to
** connect to a (possibly remote) database where the user name and
** password need to be specified.
**
** The program reads a record from the input file (possibly standard
** input), and either updates an existing record (singular with -k) or
** records (possibly plural with -K) or inserts a new record if there
** was no record with the matching key fields.  The insert can be
** attempted before the update by specifying -I instead of -U; if -S
** is specified, the record is fetched before either insert or update
** is tried.  If none of -I, -S, -U is specified, the program uses a
** simple heuristic to determine whether to try insert or update
** first.  It keeps a count of the number of rows inserted versus the
** number of rows updated and tries to do insert first if the insert
** count is greater than the update count (so it does an update first
** on the first row of data).  With the -k option, it is an error if
** the update ever modifies more than one row of data; with the -K
** option, this is permitted.  If the -q option is specified, there is
** no feedback during the load.  If the -v option is specified, then
** each record processed is reflected by an 'I' or 'U' on the screen,
** with 'X' marking rejected rows and 'C' marking commit points in a
** logged database.  If neither the -q nor the -v option is specified,
** then the program reports the number of rows processed in the input
** file every time it commits -- or reaches the number of records
** which would cause a commit in an unlogged database.  Any rejected
** records are logged in the specified log file with a verbose
** description of the problem.  If the reject file is specified, the
** raw data for the input is copied there, ready for editing and
** reloading.  The -e option specifies how many errors are tolerated
** before the program quits; the default is indefinitely large.  The
** table can be locked in shared (-s) or exclusive (-x) mode before
** any processing occurs.  The data in the load file can represent a
** subset of the columns in the table, specified by the -c option; if
** the program ever has to do an insert, the absent columns must all
** either allow nulls or have a default defined for the insert to
** succeed.  The columns specified in the -k or -K option must also be
** listed in the -c option.  The order of the column names in the -c
** option determines the order in which the fields in the data file
** are interpreted.
**
** There is at present no provision for updating the key columns
** themselves.  The probable technique for handling this will be to
** require the -c option, with the old-value data fields being marked
** (possibly with a '+') and repeated as necessary with the new
** values.  This leads to a notation such as:
**
** sqlupload -d db -t table -k col1,col3 -c +col1,col2,+col3,col1
**
** This means that the primary key for the table consists of col1 and
** col3.  The old values of col1 and col3 can be found in field 1 and
** 3 of the data file; the new value for col2 is in field 2 and the
** new value for col1 is in field 4.
**
** As with SQLCMD, the -D option allows you to override DBDELIMITER
** (default is the pipe symbol) and the -E option allows you to
** override the escape character (default is backslash).  Note that
** you need to type -E'\\' on the Unix command line to get a valid
** escape string to SQLUPLOAD.
*/

#define MAIN_PROGRAM
#define USE_JLSS_GETOPT

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "esqlc.h"
#include "context.h"
#include "getopt.h"
#include "sqlcmd.h"
#include "jlsstools.h"
#include "stderr.h"
#include "upload.h"
#include "sqlerr.h"
#include "debug.h"
#include "emalloc.h"

typedef unsigned long Ulong;

#ifndef DEF_HEURISTIC_MEMORY
#define DEF_HEURISTIC_MEMORY    11
#endif /* DEF_HEURISTIC_MEMORY */

#ifndef DEF_ROWS_PER_TX
/* DEF_ROWS_PER_TX is a string because ctxt_settransize() takes a string */
#define DEF_ROWS_PER_TX "1024"
#endif

static const char usestr[] =
    "[-Vh] -d dbase [-o owner] -t table {-k pkcols | -K cols}\n"
    "\t[-l logfile] [-r rejects] [-n rows] [-e errors] [-i skip] [-f input]\n"
    "\t[-q|v] [-s|x] [-H|I|S|U] [-c cols] [-D delim] [-E escape]\n"
    "\t[-u username -p password] [file]";

static const char opts[] = "D:E:HIK:SUVZ:c:d:e:f:hi:k:l:n:o:p:qr:st:u:vx";

static const LockMode LOCK_SHARED = "SHARE";
static const LockMode LOCK_EXCLUSIVE = "EXCLUSIVE";

int     pflag = SQLUPLOAD;

static void arg_parser(int argc, char **argv, Control *ctrl, ConnInfo *conn);
static void describe_actions(FILE *fp, const Control *ctrl, const ConnInfo *conn);
static void chk_input(const char *file);
static void set_debug(void);
static DbsType dbstype_from_conninfo(const ConnInfo *conn);

#ifndef lint
/* Prevent over-aggressive optimizers from eliminating ID string */
const char jlss_id_sqlupload_c[] = "@(#)$Id: sqlupload.c,v 2013.1 2013/09/08 23:19:13 jleffler Exp $";
#endif /* lint */

int main(int argc, char **argv)
{
    Control  ctrl = { 0 };      /* ANSI C compilers */
    ConnInfo conn = { 0 };      /* ANSI C compilers */
    int status = EXIT_SUCCESS;

    err_setarg0(argv[0]);
    set_debug();
    ctxt_init();
    conn.connname = STRDUP("sqlupload_conn");
    arg_parser(argc, argv, &ctrl, &conn);
    if (ctrl.volume == V_VERBOSE)
        describe_actions(stdout, &ctrl, &conn);

    /* Check that input file exists before connecting to database */
    chk_input(ctrl.inpfile);

    if (db_connect(&conn) != 0)
        status = EXIT_FAILURE;
    else
    {
        int rc;
        ctrl.dbs_type = dbstype_from_conninfo(&conn);
        status = upload_stmt(&ctrl);
        rc = db_disconnect(&conn);
        if (status == 0 && rc != 0)
            status = EXIT_FAILURE;
    }

    return status;
}

static void set_debug(void)
{
    char *env;
    if ((env = getenv("SQLUPLOAD_DEBUGLEVEL")) != 0)
    {
        db_setdebug(atoi(env));
    }
}

static void chk_input(const char *file)
{
    FILE *fp;

    if (file != 0)
    {
        if ((fp = fopen(file, "r")) == 0)
            err_syserr("unable to open input file %s", file);
        if (fclose(fp) != 0)
            err_syserr("unable to close input file %s", file);
    }
}

/* Convert from ConnInfo DBS type information to UPLOAD type */
static DbsType dbstype_from_conninfo(const ConnInfo *conn)
{
    DbsType dbstype = ((conn->mode_ansi) ? IS_MODE_ANSI : 0)
                    | ((conn->logged)    ? IS_LOGGED_DB : 0)
                    | ((conn->online)    ? IS_ONLINE_DB : 0);
    return dbstype;
}

/*
** validate_number() - check for non-negative number
**
** Although final result will be assigned to a size_t, the expected
** value ranges should not exceed INT_MAX.  The values validated are:
** - number of rows per transaction
** - number of rows to skip before processing data
** - number of errors to accept before stopping
*/
static int validate_number(const char *str, char opt)
{
    long val;
    char *end;

    errno = 0;
    val = strtol(str, &end, 0);
    if (*end != '\0' || val <= 0 || val > INT_MAX || errno != 0)
    {
        err_remark("argument to -%c (%s) is not a positive number (1..%d).\n",
                   opt, str, INT_MAX);
        val = -1;
    }
    return(val);
}

static int chk_option(char **optval, int opt, char *arg)
{
    int rv = 0;

    if (*optval != 0)
    {
        err_remark("-%c option repeated on command line.\n", opt);
        rv = 1;
    }
    *optval = arg;
    return rv;
}

static void upld_version(const char *name, const char *ver)
{
    const char *v;
    char buffer[128];

    v = err_rcs_string(ver, buffer, sizeof(buffer));
    err_logmsg(stdout, ERR_ERR, EXIT_SUCCESS, "%s Version %s\n%s\n",
                name, v, esqlc_version());
}

static void arg_parser(int argc, char **argv, Control *ctrl, ConnInfo *conn)
{
    char    *Kflag = 0;
    char    *kflag = 0;
    char    *eflag = 0;
    char    *iflag = 0;
    char    *nflag = 0;
    int      sflag = 0;
    int      xflag = 0;
    int      qflag = 0;
    int      vflag = 0;
    int      errs = 0;
    char    *env;
    int      opt;
    long     rows_per_tx;

    ctrl->delim = DEF_DELIM;
    ctrl->escape = DEF_ESCAPE;
    ctrl->hmode = H_UNDEFINED;

    if ((env = getenv("DBDELIMITER")) != 0)
        ctrl->delim = env;
    if ((env = getenv("DBESCAPE")) != 0)
        ctrl->escape = env;

    while ((opt = GETOPT(argc, argv, opts)) != EOF)
    {
        switch (opt)
        {
        case 'D':
            ctrl->delim = optarg;
            break;
        case 'E':
            ctrl->escape = optarg;
            break;
        case 'H':
        case 'I':
        case 'S':
        case 'U':
            if (ctrl->hmode == char_to_HMode((char)opt))
            {
                err_remark("-%c flag repeated on command line.\n", opt);
                errs++;
            }
            else if (ctrl->hmode != H_UNDEFINED)
            {
                err_remark("you cannot specify both -%c and -%c on the "
                           "command line.\n", (int)ctrl->hmode, opt);
                errs++;
            }
            else
                ctrl->hmode = char_to_HMode((char)opt);
            break;
        case 'K':
            errs += chk_option(&Kflag, opt, optarg);
            break;
        case 'V':
            if (errs == 0)
            {
                upld_version("SQLUPLOAD", &"@(#)$Revision: 2013.1 $ ($Date: 2013/09/08 23:19:13 $)"[4]);
                /* NOTREACHED */
            }
            break;
        case 'c':
            errs += chk_option(&ctrl->columns, opt, optarg);
            break;
        case 'd':
            errs += chk_option(&conn->database, opt, optarg);
            break;
        case 'e':
            errs += chk_option(&eflag, opt, optarg);
            break;
        case 'f':
            errs += chk_option(&ctrl->inpfile, opt, optarg);
            break;
        case 'h':
            err_usage(usestr);
            break;
        case 'i':
            errs += chk_option(&iflag, opt, optarg);
            break;
        case 'k':
            errs += chk_option(&kflag, opt, optarg);
            break;
        case 'l':
            errs += chk_option(&ctrl->logfile, opt, optarg);
            break;
        case 'n':
            errs += chk_option(&nflag, opt, optarg);
            break;
        case 'o':
            errs += chk_option(&ctrl->owner, opt, optarg);
            break;
        case 'p':
            /* Duplicate password - so original can be overwritten */
            errs += chk_option(&conn->password, opt, STRDUP(optarg));
            /*
            ** Overwrite password to reduce the chance of it being seen
            ** via ps, but it does not do anything useful on Solaris 7.
            */
            memset(optarg, 'X', strlen(optarg));
            break;
        case 'q':
            if (qflag != 0)
            {
                err_remark("-%c flag repeated on command line.\n", opt);
                errs++;
            }
            qflag = 1;
            break;
        case 'r':
            errs += chk_option(&ctrl->rejfile, opt, optarg);
            break;
        case 's':
            if (sflag != 0)
            {
                err_remark("-%c flag repeated on command line.\n", opt);
                errs++;
            }
            sflag = 1;
            break;
        case 't':
            errs += chk_option(&ctrl->table, opt, optarg);
            break;
        case 'u':
            errs += chk_option(&conn->username, opt, optarg);
            break;
        case 'v':
            if (vflag != 0)
            {
                err_remark("-%c flag repeated on command line.\n", opt);
                errs++;
            }
            vflag = 1;
            break;
        case 'x':
            if (xflag != 0)
            {
                err_remark("-%c flag repeated on command line.\n", opt);
                errs++;
            }
            xflag = 1;
            break;
        case 'Z':
            /* If you specify a non-numeric argument, tough! */
            db_setdebug(atoi(optarg));
            break;
        default:
            errs++;
            break;
        }
    }

    if (conn->database == 0)
    {
        err_remark("you did not specify which database with the -d option\n");
        errs++;
    }
    if (ctrl->table == 0)
    {
        err_remark("you did not specify which table with the -t option\n");
        errs++;
    }
    /* - Provide support for SQLCMDPASSWORDS!
    if ((conn->password != 0 && conn->username == 0) ||
        (conn->password == 0 && conn->username != 0))
    {
        err_remark("you did not specify both username and password\n");
        errs++;
    }
    */
    if (kflag != 0 && Kflag != 0)
    {
        err_remark("you cannot specify both -k and -K on the command line\n");
        errs++;
    }
    if (kflag == 0 && Kflag == 0)
    {
        /* Constraint to be removed: requires DB connection to */
        /* validate that table has PK or single unique index.  */
        err_remark("you must either specify the primary key with -k or a "
                "non-unique key with -K\n");
        errs++;
    }
    if (sflag && xflag)
    {
        err_remark("you cannot specify both SHARED lock with -s and EXCLUSIVE lock with -x\n");
        errs++;
    }
    if (qflag && vflag)
    {
        err_remark("you cannot specify both quiet with -q and verbose with -v\n");
        errs++;
    }
    if (ctrl->inpfile != 0 && optind != argc)
    {
        err_remark("you cannot specify both the input file with -f and\n"
                "\ta file name after the other arguments\n");
        errs++;
    }
    if (ctrl->inpfile == 0 && optind < argc - 1)
    {
        err_remark("you can only specify one input file on the command line\n");
        errs++;
    }

    if (eflag)
    {
        if ((ctrl->stop_after = validate_number(eflag, 'e')) == (size_t)-1)
            errs++;
    }

    if (iflag)
    {
        if ((ctrl->skip_first = validate_number(iflag, 'i')) == (size_t)-1)
            errs++;
    }

    if (!nflag)
    {
        static char def_nflag[] = DEF_ROWS_PER_TX;  /*=C++=*/
        nflag = def_nflag;
    }
    if ((rows_per_tx = validate_number(nflag, 'n')) < 0)
        errs++;
    else
        ctrl->txsize = nflag;

    if (vflag)
    {
        assert(qflag == 0);
        ctrl->volume = V_VERBOSE;
    }
    else if (qflag)
    {
        assert(vflag == 0);
        ctrl->volume = V_QUIET;
        ctxt_setsilence("on");
    }

    ctrl->lockmode = (sflag) ? LOCK_SHARED : LOCK_EXCLUSIVE;

    if (ctrl->hmode == H_UNDEFINED)
        ctrl->hmode = H_HEURISTIC;
    set_heuristic(ctrl->hmode, DEF_HEURISTIC_MEMORY);

    if (kflag)
    {
        ctrl->keycols = kflag;
        ctrl->keytype = K_UNIQUE;
    }
    else if (Kflag)
    {
        ctrl->keycols = Kflag;
        ctrl->keytype = K_DUPS;
    }
    else
    {
        ctrl->keytype = K_UNDEF;
        ctrl->keycols = 0;
    }

    /* You must do UPDATE or SELECT before INSERT with a K_DUPS key */
    /* The INSERT should always succeed, of course */
    if (ctrl->keytype == K_DUPS &&
        (ctrl->hmode == H_HEURISTIC || ctrl->hmode == H_INSERT))
    {
        err_remark("You must use either -U or -S when you use -K\n");
        errs++;
    }

    if (ctrl->inpfile == 0 && optind < argc)
    {
        ctrl->inpfile = argv[optind++];
    }

    if (errs)
        err_usage(usestr);
}

static void print_connection(FILE *fp, const ConnInfo *conn)
{
    fprintf(fp, "Connection Information:\n");
    if (conn->database)
        fprintf(fp, "database: %s\n", conn->database);
    if (conn->username)
        fprintf(fp, "username: %s\n", conn->username);
    if (conn->password)
        fprintf(fp, "password: %s\n", conn->password);
}

/* Describe what we're going to do */
static void describe_actions(FILE *fp, const Control *ctrl, const ConnInfo *conn)
{

    print_connection(fp, conn);

    fprintf(fp, "Upload Control Information:\n");
    if (ctrl->owner)
        fprintf(fp, "owner:    %s\n", ctrl->owner);
    fprintf(fp, "table:    %s\n", ctrl->table);

    fprintf(fp, "%s: %s\n", (ctrl->keytype == K_UNIQUE) ? "unique key columns" :
            "non-unique columns", ctrl->keycols);

    if (ctrl->columns)
        fprintf(fp, "data columns in file: %s\n", ctrl->columns);

    fprintf(fp, "input data file:  %s\n", ctrl->inpfile ? ctrl->inpfile :
            "standard input");

    if (ctrl->logfile)
        fprintf(fp, "error log file:   %s\n", ctrl->logfile);
    if (ctrl->rejfile)
        fprintf(fp, "reject data file: %s\n", ctrl->rejfile);

    fprintf(fp, "table locking: %s\n", ctrl->lockmode);
    fprintf(fp, "error rows before stopping: %lu\n", (Ulong)ctrl->stop_after);
    fprintf(fp, "rows to skip: %lu\n", (Ulong)ctrl->skip_first);
    fprintf(fp, "rows per transaction: %s\n", ctrl->txsize);

    fprintf(fp, "Volume: ");
    switch (ctrl->volume)
    {
    case V_VERBOSE:
        fprintf(fp, "verbose mode\n");
        break;
    case V_QUIET:
        fprintf(fp, "quiet mode\n");
        break;
    case V_NORMAL:
        fprintf(fp, "normal mode\n");
        break;
    }

    fprintf(fp, "Heuristic mode: ");
    switch (ctrl->hmode)
    {
    case H_INSERT:
        fprintf(fp, "do INSERT and then UPDATE\n");
        break;
    case H_UPDATE:
        fprintf(fp, "do UPDATE and then INSERT\n");
        break;
    case H_SELECT:
        fprintf(fp, "use SELECT to determine whether to do INSERT or UPDATE\n");
        break;
    case H_HEURISTIC:
        fprintf(fp, "use heuristic to decide on UPDATE/INSERT order\n");
        break;
    case H_UNDEFINED:
    default:
        assert(0);
        break;
    }

    fprintf(fp, "Delimiter = %c (%d)\n", *ctrl->delim, *ctrl->delim);
    fprintf(fp, "Escape = %c (%d)\n", *ctrl->escape, *ctrl->escape);
}

void sql_longjmp(int status)
{
    exit(1);
}
