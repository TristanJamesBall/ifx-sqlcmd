/*
@(#)File:           $RCSfile: context.c,v $
@(#)Version:        $Revision: 2016.1 $
@(#)Last changed:   $Date: 2016/07/08 01:22:38 $
@(#)Purpose:        Context Handling for SQLCMD
@(#)Author:         J Leffler
@(#)Copyright:      (C) JLSS 1992-93,1995,1997-2006,2008,2010-12,2015-16
@(#)Product:        SQLCMD Version 90.02 (2016-07-28)
*/

/*TABSTOP=4*/

/* -- Include Files */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <assert.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "context.h"
#include "cchrstrlit.h"
#include "datefmt.h"
#include "emalloc.h"
#include "history.h"
#include "jlss.h"
#include "jlsstools.h"
#include "readcmd.h"
#include "sqlcmd.h"
#include "sqlerr.h"
#include "stderr.h"

/* -- Constant Definitions */

#define global  /* Variable defined here for use elsewhere */

#if CTXT_STACKSIZE <= 0
#undef CTXT_STACKSIZE
#endif

#ifndef CTXT_STACKSIZE
#define CTXT_STACKSIZE 10
#endif /* CTXT_STACKSIZE */

#define CLOSE_NONE      0x0
#define CLOSE_INPUT     0x1
#define CLOSE_OUTPUT    0x2
#define CLOSE_ERROR     0x4

#define MAX_EOR         8       /* Maximum total length of EOR string, inc NUL */

/* -- Macro Definitions */

#define NIL(x)      ((x)0)

/* -- Type Definitions */

typedef struct Stack
{
    short           sp;
    Uchar           stack[CTXT_STACKSIZE];
} Stack;

/*
** Note that the SKIP (ctxt_[gs]etskipsize()) is more than a little
** peculiar.  It can be set on the command line (for SQLRELOAD), but
** otherwise is only set for the scope of a LOAD or RELOAD statement.
** Hence it is not an internal command and is not reported.
*/

/*
** General question: should date, blobdir, quote, escape, etc all be
** handled with Stacks, somehow?  Not sure.  Since format can be
** stacked, the XML record tag should be stacked along with the format
** (so that format push; format xml 'tag1'; ... ; format push; format
** xml 'tag2'; ...; format pop; works correctly, restoring tag1).
** This does not happen at the moment: BUG!
*/

struct Context
{
    FILE           *input;      /* Input channel */
    char           *infile;     /* Name of input file */
    size_t          linenum;    /* Input line number in input file */
    FILE           *output;     /* Output channel */
    char           *outfile;    /* Name of output file */
    FILE           *error;      /* Error channel */
    char           *errfile;    /* Name of error file */
    int             flags;      /* Channel flags */
    char           *date;       /* DATE format */
    char            quote;      /* Quote character */
    char            escape;     /* Escape character */
    char            delim;      /* Delimiter character */
    char            eor[MAX_EOR];   /* End of Record string (normally "\n") */
    int             tsize;      /* Transaction size */
    int             qlimit;     /* Maximum rows returned from SELECT */
    int             skip;       /* Number of rows to skip on LOAD, RELOAD */
    int             ibase;      /* Input base for integers (0, 8, 10, 16) */
    int             load_ctxt;  /* Is this a (RE)LOAD context */
    Stack           format;     /* Output format stack */
    Stack           history;    /* History mode stack */
    Stack           heading;    /* Column names mode stack */
    Stack           hexmode;    /* DB-Access hex (-X) mode stack */
    Stack           nonstop;    /* Continue mode stack */
    Stack           silence;    /* Silence mode stack */
    Stack           single;     /* Single-step mode stack */
    Stack           trace;      /* Trace mode stack */
    Stack           types;      /* Type names mode stack */
    Stack           verbose;    /* Verbosity mode stack */
    Stack           bmark;      /* Benchmark mode stack */
    char           *blobdir;    /* Blob directory */
    char           *xmlrecordtag;   /* XML record tag */
    char           *xmlrecsettag;   /* XML record set tag */
    char           *xmlheadertag;   /* XML header tag */
};
typedef struct Context Context;

/* -- Declarations */

static Context *context;
static int      ctxt_num;

/* op_on used in sqlcmd.c; others declared external for consistency */
static const char op_none[] = "<none>";
global const char op_off[] = "off";
global const char op_on[] = "on";
global const char op_pop[] = "pop";
global const char op_push[] = "push";
global const char op_toggle[] = "toggle";

/* Output formats */
static const char op_csv[] = "csv";
static const char op_markdown[] = "markdown";
static const char op_fixed[] = "fixed";
static const char op_fixsep[] = "fixsep";
static const char op_fixdel[] = "fixdel";
static const char op_quote[] = "quote";
static const char op_select[] = "select";
global const char op_unload[] = "unload";   /* op_unload used in internal.c */
static const char op_db2[] = "db2";
static const char op_xml[] = "xml";
static const char op_html[] = "html";
global const char op_block[] = "block";     /* op_block used in sqlcmd.c */

/* Keep this list in sync with enum Operator in context.h */
static const char *const op_list[] =
{
    op_none,
    op_off,
    op_on,
    op_pop,
    op_push,
    op_quote,
    op_select,
    op_toggle,
    op_unload,
    op_fixed,
    op_fixsep,
    op_fixdel,
    op_csv,
    op_db2,
    op_xml,
    op_html,
    op_block,
};

#ifndef lint
/* Prevent over-aggressive optimizers from eliminating ID string */
extern const char jlss_id_context_c[];
const char jlss_id_context_c[] = "@(#)$Id: context.c,v 2016.1 2016/07/08 01:22:38 jleffler Exp $";
#endif /* lint */

static void ctxt_print_str(const char *tag, int taglen, const char *str, size_t len)
{
    char buff1[32];
    char buff2[32];

    nstrcpy(buff2, len + 1, str);
    str_cstrlit(buff2, buff1, sizeof(buff1));
    fprintf(ctxt_output(), "%-*s '%s'\n", taglen, tag, buff1);
}

static int ctxt_atoi(const char *s, const char *t, int min, Error err)
{
    int             i;

    i = atoi(s);
    if (i < min)
    {
        cmd_warning(err, t);
        i = min;
    }
    return(i);
}

/* Convert a quoted string such as "\r\n" into a nul-terminated string */
/* JL 2001-09-10: Minor break in backwards compatibility - insist on quotes! */
static int ctxt_quotestring(const char *s, char *buffer, size_t buflen)
{
    int             c;
    int             q;
    const char     *t = s;

    c = *t++;
    if (c == '\'' || c == '"')
    {
        const char *start = t;
        int rc;
        /* Isolate quoted string */
        q = c;
        while ((c = *t++) != '\0' && c != q)
        {
            if (c == '\\')
            {
                if (*t == '\0')
                {
                    cmd_warning(E_INVSTRLIT, s);
                    return(-1);
                }
                t++;
            }
        }
        if (c != q)
        {
            cmd_warning(E_INVCLQUOTE, s);
            return(-1);
        }
        assert((t - 1) > start);
        /* Convert from C String Literal to string */
        if ((rc = cstrlit_str(start, t - 1, buffer, buflen)) <= 0)
        {
            if (rc == -1)
                cmd_warning(E_INVSTRLIT, s);
            else
                cmd_warning(E_STRTOOLONG, s);
            return(-1);
        }
    }
    else
    {
        cmd_warning(E_INVOPQUOTE, s);
        return(-1);
    }
    return(0);
}

static int ctxt_quotechar(const char *s, char *c)
{
    char            buffer[2];
    int             rv;

    if ((rv = ctxt_quotestring(s, buffer, sizeof(buffer))) == 0)
        *c = buffer[0];
    return(rv);
}

static Operator ctxt_opcon(const char *s)
{
    Operator        op;

    if (*s == '\0')
        op = OP_NONE;
    else if (cistrcmp(s, op_on) == 0)
        op = OP_ON;
    else if (cistrcmp(s, op_off) == 0)
        op = OP_OFF;
    else if (cistrcmp(s, op_pop) == 0)
        op = OP_POP;
    else if (cistrcmp(s, op_push) == 0)
        op = OP_PUSH;
    else if (cistrcmp(s, op_toggle) == 0)
        op = OP_TOGGLE;
    else if (cistrcmp(s, op_unload) == 0)
        op = OP_UNLOAD;
    else if (cistrcmp(s, op_select) == 0)
        op = OP_SELECT;
    else if (cistrcmp(s, op_fixed) == 0)
        op = OP_FIXED;
    else if (cistrcmp(s, op_fixdel) == 0)
        op = OP_FIXDEL;
    else if (cistrcmp(s, op_fixsep) == 0)
        op = OP_FIXSEP;
    else if (cistrcmp(s, op_csv) == 0)
        op = OP_CSV;
    else if (cistrcmp(s, op_markdown) == 0)
        op = OP_MARKDOWN;
    else if (cistrcmp(s, op_db2) == 0)
        op = OP_DB2;
    else if (cistrcmp(s, op_xml) == 0)
        op = OP_XML;
    else if (cistrcmp(s, op_html) == 0)
        op = OP_HTML;
    else if (cistrcmp(s, op_quote) == 0)
        op = OP_QUOTE;
    else if (cistrcmp(s, op_block) == 0)
        op = OP_BLOCK;
    else
    {
        cmd_warning(E_INVOPERATOR, s);
        op = OP_NONE;
    }

    return(op);
}

static void ctxt_stack(Stack *s, Operator op, const char *m)
{
    int             sp;

    switch (op)
    {
    case OP_ON:
    case OP_OFF:
    case OP_SELECT:
    case OP_UNLOAD:
    case OP_FIXED:
    case OP_FIXDEL:
    case OP_FIXSEP:
    case OP_QUOTE:
    case OP_CSV:
	case OP_MARKDOWN:
    case OP_XML:
    case OP_HTML:
    case OP_BLOCK:
        /* Set current state */
        s->stack[s->sp] = (char)op;
        break;
    case OP_PUSH:
        /* Create new current state with same value as old state */
        if (s->sp >= CTXT_STACKSIZE)
            cmd_warning(E_CTXTOVERFLOW, m);
        else
        {
            sp = s->sp++;
            s->stack[sp + 1] = s->stack[sp];
        }
        break;
    case OP_POP:
        /* Restore previous state */
        if (s->sp <= 0)
            cmd_warning(E_CTXTUNDERFLOW, m);
        else
            s->sp--;
        break;
    case OP_TOGGLE:
        if (s->stack[s->sp] == OP_ON)
            s->stack[s->sp] = (char)OP_OFF;
        else
            s->stack[s->sp] = (char)OP_ON;
        break;
    case OP_NONE:
        fprintf(ctxt_output(), "%s: %s\n", m, op_list[s->stack[s->sp]]);
        break;
    default:
        err_error("uncoded context operation %d in " __FILE__ "\n", (int)op);
        /* NOTREACHED */
        break;
    }
}

static int ctxt_peep(Stack *s)
{
    return(s->stack[s->sp]);
}

static Context *ctxt_getcontext(void)
{
    if (context == NIL(Context *))
        err_error("null context in " __FILE__ "\n");
    assert(ctxt_num > 0);
    return(&context[ctxt_num - 1]);
}

static char *ctxt_envblobdir(void)
{
    const char *blobdir;

    if (((blobdir = getenv("DBTEMP")) == NIL(char *)) &&
        ((blobdir = getenv("TMPDIR")) == NIL(char *)))
        blobdir = DEF_TMPDIR;
    return(STRDUP(blobdir));
}

static char *ctxt_envdate(void)
{
    return(STRDUP(fmt_dbdate()));
}

static int ctxt_envchar(const char *var, const char *def)
{
    const char *cp;

    if ((cp = getenv(var)) == NIL(char *))
        cp = def;
    return(*cp);
}

static const char *ctxt_envstr(const char *var, const char *def)
{
    const char *cp;

    if ((cp = getenv(var)) == NIL(char *))
        cp = def;
    return(cp);
}

static void ctxt_release(Context *cp)
{
    FREE(cp->date);
    FREE(cp->infile);
    FREE(cp->outfile);
    FREE(cp->errfile);
    if (cp->flags & CLOSE_INPUT)
        fclose(cp->input);
    if (cp->flags & CLOSE_OUTPUT)
        fclose(cp->output);
    if (cp->flags & CLOSE_ERROR)
        fclose(cp->error);
}

void ctxt_recover(void)
{
    while (ctxt_num > 1)
        ctxt_endcontext();
}

void ctxt_free(void)
{
    int             i;

    if (context != NIL(Context *))
    {
        for (i = 0; i < ctxt_num; i++)
            ctxt_release(&context[i]);
        FREE(context);
        ctxt_num = 0;
    }
}

static void ctxt_initstack(Stack *s, int val)
{
    s->sp = 0;
    s->stack[0] = val;
}

void ctxt_init(void)
{
    /* Release previous context(s) -- if any */
    ctxt_free();

    /* Create new context */
    context = (Context *)MALLOC(sizeof(Context));
    ctxt_num = 1;

    /* Initialise context */
    context[0].input = stdin;
    context[0].infile = STRDUP("/dev/stdin");
    context[0].linenum = 0;
    context[0].output = stdout;
    context[0].outfile = STRDUP("/dev/stdout");
    context[0].error = stderr;
    context[0].errfile = STRDUP("/dev/stderr");
    context[0].flags = CLOSE_NONE;
    context[0].quote = ctxt_envchar("DBQUOTE", DEF_QUOTE);
    context[0].delim = ctxt_envchar("DBDELIMITER", DEF_DELIM);
    context[0].escape = ctxt_envchar("DBESCAPE", DEF_ESCAPE);
    context[0].date = ctxt_envdate();
    context[0].blobdir = ctxt_envblobdir();
    context[0].tsize = DEF_TRANSIZE;
    context[0].ibase = DEF_IBASE;
    context[0].qlimit = DEF_QUERYLIMIT;
    context[0].skip = 0;
    context[0].load_ctxt = 0;
    context[0].xmlrecordtag = STRDUP("record");
    context[0].xmlrecsettag = STRDUP("sqlcmd_record_set");
    context[0].xmlheadertag = STRDUP("header");

    strncpy(context[0].eor, ctxt_envstr("DBNEWLINE", "\n"),MAX_EOR - 1);
	
    ctxt_initstack(&context[0].format, OP_SELECT);
    ctxt_initstack(&context[0].heading, OP_OFF);
    ctxt_initstack(&context[0].history, OP_OFF);
    ctxt_initstack(&context[0].nonstop, OP_OFF);
    ctxt_initstack(&context[0].silence, OP_OFF);
    ctxt_initstack(&context[0].single, OP_OFF);
    ctxt_initstack(&context[0].trace, OP_OFF);
    ctxt_initstack(&context[0].types, OP_OFF);
    ctxt_initstack(&context[0].verbose, OP_OFF);
    ctxt_initstack(&context[0].bmark, OP_OFF);
    ctxt_initstack(&context[0].hexmode, OP_OFF);
}

static void ctxt_resetstack(Stack *s)
{
    s->stack[0] = s->stack[s->sp];
    s->sp = 0;
}

char *ctxt_loadfile(void)
{
    return(ctxt_getcontext()->infile);
}

char *ctxt_infile(void)
{
    const Context *curr = ctxt_getcontext();
    if (curr->load_ctxt != 0)
        curr--;
    return(curr->infile);
}

char *ctxt_outfile(void)
{
    return(ctxt_getcontext()->outfile);
}

char *ctxt_errfile(void)
{
    return(ctxt_getcontext()->errfile);
}

FILE *ctxt_input(void)
{
    return(ctxt_getcontext()->input);
}

FILE *ctxt_output(void)
{
    return(ctxt_getcontext()->output);
}

FILE *ctxt_error(void)
{
    return(ctxt_getcontext()->error);
}

/* Create new context */
/*
** NB: when used, the calling code always does a variant of
** ctxt_setinput() and follows through to ctxt_endcontext().
** Hence, linenum and infile is handled in ctxt_setinput().
*/
void ctxt_newcontext(void)
{
    context = (Context *)REALLOC(context, sizeof(Context) * (ctxt_num + 1));
    context[ctxt_num] = context[ctxt_num - 1];
    context[ctxt_num].flags = CLOSE_NONE;
    context[ctxt_num].infile = STRDUP(context[ctxt_num].infile);
    context[ctxt_num].outfile = STRDUP(context[ctxt_num].outfile);
    context[ctxt_num].errfile = STRDUP(context[ctxt_num].errfile);
    context[ctxt_num].date = STRDUP(context[ctxt_num].date);
    context[ctxt_num].blobdir = STRDUP(context[ctxt_num].blobdir);
    context[ctxt_num].xmlrecordtag = STRDUP(context[ctxt_num].xmlrecordtag);
    context[ctxt_num].xmlrecsettag = STRDUP(context[ctxt_num].xmlrecsettag);
    context[ctxt_num].xmlheadertag = STRDUP(context[ctxt_num].xmlheadertag);
    context[ctxt_num].load_ctxt = 0;
    ctxt_resetstack(&context[ctxt_num].format);
    ctxt_resetstack(&context[ctxt_num].heading);
    ctxt_resetstack(&context[ctxt_num].history);
    ctxt_resetstack(&context[ctxt_num].nonstop);
    ctxt_resetstack(&context[ctxt_num].silence);
    ctxt_resetstack(&context[ctxt_num].single);
    ctxt_resetstack(&context[ctxt_num].trace);
    ctxt_resetstack(&context[ctxt_num].types);
    ctxt_resetstack(&context[ctxt_num].verbose);
    ctxt_resetstack(&context[ctxt_num].bmark);
    ctxt_resetstack(&context[ctxt_num].hexmode);
    ctxt_num++;
}

int ctxt_setloadinput(const char *str, LoadDest dest, FileMode mode)
{
    int rc;

    if (dest == LD_PIPE)
    {
        FILE *fp;
        if ((fp = sql_epopen(str, "r")) != NIL(FILE *))
            ctxt_newinput(fp, str);
        rc = (fp == NIL(FILE *)) ? -1 : 0;
    }
    else
        rc = ctxt_setinput(str);
    ctxt_getcontext()->load_ctxt = 1;
    return(rc);
}

int ctxt_setinput(const char *fn)
{
    FILE *fp;

    /* Special case: file name of '-' means standard input */
    if (strcmp(fn, "-") == 0)
        fp = stdin;
    else
        fp = sql_efopen(fn, "rb");

    if (fp != NIL(FILE *))
        ctxt_newinput(fp, fn);

    return((fp == NIL(FILE *)) ? -1 : 0);
}

static int ctxt_setoutput_mode(const char *fn, const char *fm)
{
    FILE *fp;

    if ((fp = sql_efopen(fn, fm)) != NIL(FILE *))
        ctxt_newoutput(fp, fn);
    return((fp == NIL(FILE *)) ? -1 : 0);
}

int ctxt_setoutput(const char *fn)
{
    int rc = ctxt_setoutput_mode(fn, "wb");
    return(rc);
}

int ctxt_setunloadoutput(const char *str, LoadDest dest, FileMode mode)
{
    int rc;
    if (dest == LD_PIPE)
    {
        FILE *fp;

        if ((fp = sql_epopen(str, "w")) != NIL(FILE *))
            ctxt_newoutput(fp, str);
        rc = ((fp == NIL(FILE *)) ? -1 : 0);
    }
    else
    {
        const char *fm = (mode == FM_APPEND) ? "ab" : "wb";
        rc = ctxt_setoutput_mode(str, fm);
    }
    return(rc);
}

int ctxt_seterror(const char *fn)
{
    FILE *fp;

    if ((fp = sql_efopen(fn, "w")) != NIL(FILE *))
        ctxt_newerror(fp, fn);
    return((fp == NIL(FILE *)) ? -1 : 0);
}

void ctxt_newinput(FILE *fp, const char *fn)
{
    Context        *cp = ctxt_getcontext();

    if (cp->flags & CLOSE_INPUT)
        fclose(cp->input);
    cp->flags &= ~CLOSE_INPUT;
    FREE(cp->infile);
    cp->input = fp;
    cp->infile = STRDUP(fn);
    if (fp != stdin)
        cp->flags |= CLOSE_INPUT;
    cp->linenum = 1;
    cmd_set_linenum(cp->linenum);
}

void ctxt_newoutput(FILE *fp, const char *fn)
{
    Context        *cp = ctxt_getcontext();

    if (cp->flags & CLOSE_OUTPUT)
        fclose(cp->output);
    cp->flags &= ~CLOSE_OUTPUT;
    FREE(cp->outfile);
    cp->output = fp;
    cp->outfile = STRDUP(fn);
    if (fp != stdout && fp != stderr)
        cp->flags |= CLOSE_OUTPUT;
}

void ctxt_newerror(FILE *fp, const char *fn)
{
    Context        *cp = ctxt_getcontext();

    if (cp->flags & CLOSE_ERROR)
        fclose(cp->error);
    cp->flags &= ~CLOSE_ERROR;
    FREE(cp->errfile);
    cp->error = fp;
    cp->errfile = STRDUP(fn);
    if (fp != stderr && fp != stdout)
        cp->flags |= CLOSE_ERROR;
}

void ctxt_endcontext(void)
{
    Context        *cp = ctxt_getcontext();

    assert(ctxt_num > 0);
    ctxt_release(cp);
    ctxt_num--;
    context = (Context *)REALLOC(context, sizeof(Context) * ctxt_num);
    cmd_set_linenum(ctxt_getcontext()->linenum);
}

/**************************\
** SET CONTEXT PROPERTIES **
\**************************/

void ctxt_setlinenum(size_t linenum)
{
    Context *cp = ctxt_getcontext();
    cp->linenum = linenum;
}

void ctxt_setblobdir(const char *dir)
{
    Context        *cp;

    cp = ctxt_getcontext();
    FREE(cp->blobdir);
    cp->blobdir = STRDUP(dir);
}

void ctxt_setcontinue(const char *op)
{
    ctxt_stack(&ctxt_getcontext()->nonstop, ctxt_opcon(op), "Continue");
}

void ctxt_setdate(const char *fmt)
{
    Context        *cp;

    cp = ctxt_getcontext();
    FREE(cp->date);
    cp->date = STRDUP(fmt);
}

/* Set EOR (end of record) character string */
/* Allow \n for delim and eor, but do not allow eor[0] to equal quote or escape */
void ctxt_seteor(const char *new_eor)
{
    char            c;
    char            eor[MAX_EOR];
    Context        *cp = ctxt_getcontext();

    if (*new_eor == '\0' || *new_eor == ';')
        ctxt_print_str("EOR:", sizeof("EOR")-1, cp->eor, strlen(cp->eor));
    else if (ctxt_quotestring(new_eor, eor, sizeof(eor)) != 0)
        ;   /* Error has been reported */
    else if ((c = *eor) == cp->quote || c == cp->escape || c == cp->delim)
        cmd_warning(E_CHARCONFLICT, new_eor);
    else
        nstrcpy(cp->eor, sizeof(cp->eor), eor);
}

void ctxt_setdelim(const char *delim)
{
    char            c;
    Context        *cp = ctxt_getcontext();

    if (*delim == '\0' || *delim == ';')
        ctxt_print_str("Delimiter:", sizeof("Delimiter:")-1, &cp->delim, sizeof(cp->delim));
    else if (ctxt_quotechar(delim, &c) != 0)
        ; /* Error has been reported */
    else if (c == cp->escape || c == cp->quote)
        cmd_warning(E_CHARCONFLICT, delim);
    else
        cp->delim = c;
}

void ctxt_setescape(const char *escape)
{
    char            c;
    Context        *cp = ctxt_getcontext();

    if (*escape == '\0' || *escape == ';')
        ctxt_print_str("Escape:", sizeof("Escape:")-1, &cp->escape, sizeof(cp->escape));
    else if (ctxt_quotechar(escape, &c) != 0)
        ;   /* Error has been reported */
    else if (c == cp->delim || c == cp->quote)
        cmd_warning(E_CHARCONFLICT, escape);
    else
        cp->escape = c;
}

void ctxt_setquote(const char *quote)
{
    char            c;
    Context        *cp = ctxt_getcontext();

    if (*quote == '\0' || *quote == ';')
        ctxt_print_str("Quote:", sizeof("Quote:")-1, &cp->quote, sizeof(cp->quote));
    else if (ctxt_quotechar(quote, &c) != 0)
        ;   /* Error has been reported */
    else if (c == cp->delim || c == cp->escape)
        cmd_warning(E_CHARCONFLICT, quote);
    else
        cp->quote = c;
}

void ctxt_setformat(const char *op)
{
    Operator optype = ctxt_opcon(op);

    if (optype == OP_CSV)
    {
        char buffer[10];
        ctxt_setdelim(quotify(",", buffer, sizeof(buffer)));
        optype = OP_QUOTE;      /* Over-zealous at quoting */
    }
    if (optype == OP_DB2)
    {
        /* DB2 - requires DATE to format like DATETIME YEAR TO SECOND, */
        /* and neither DATE and DATETIME values are enclosed in quotes */
        /* NB: DATETIME YEAR TO DAY, DATETIME HOUR TO SECOND/FRACTION, */
        /* DATETIME YEAR TO SECOND/FRACTION are supported in DB2 only. */
        /* Numeric values are not enclosed in quotes either, of course */
        char buffer[10];
        ctxt_setdelim(quotify(",", buffer, sizeof(buffer)));
        ctxt_setdate("yyyy-mm-dd");
    }
    ctxt_stack(&ctxt_getcontext()->format, optype, "Format");
    if (optype == OP_NONE && ctxt_getformat() == OP_XML)
    {
        fprintf(ctxt_output(), "XML Record Set Tag: %s\n", ctxt_getxmlrecsettag());
        fprintf(ctxt_output(), "XML Record Tag:     %s\n", ctxt_getxmlrecordtag());
        fprintf(ctxt_output(), "XML Header Tag:     %s\n", ctxt_getxmlheadertag());
    }
}

void ctxt_setheading(const char *op)
{
    ctxt_stack(&ctxt_getcontext()->heading, ctxt_opcon(op), "Heading");
}

void ctxt_sethexmode(const char *op)
{
    ctxt_stack(&ctxt_getcontext()->hexmode, ctxt_opcon(op), "Hex Mode");
}

void ctxt_sethistory(const char *op)
{
    ctxt_stack(&ctxt_getcontext()->history, ctxt_opcon(op), "History");
}

void ctxt_setsilence(const char *op)
{
    ctxt_stack(&ctxt_getcontext()->silence, ctxt_opcon(op), "Silence");
}

void ctxt_setsingle(const char *op)
{
    ctxt_stack(&ctxt_getcontext()->single, ctxt_opcon(op), "Single");
}

/* Setting benchmark on implies setting trace on too */
void ctxt_setbmark(const char *op)
{
    ctxt_stack(&ctxt_getcontext()->bmark, ctxt_opcon(op), "Benchmark");
    ctxt_settrace(op);
}

void ctxt_settrace(const char *op)
{
    ctxt_stack(&ctxt_getcontext()->trace, ctxt_opcon(op), "Trace");
}

void ctxt_setqlimit(const char *sz)
{
    if (*sz == '\0' || *sz == ';')
        fprintf(ctxt_output(), "Query limit: %d\n", ctxt_getcontext()->qlimit);
    else
        ctxt_getcontext()->qlimit = ctxt_atoi(sz, "qlimit", 0, E_INVQLIMIT);
}

void ctxt_setskipsize(const char *sz)
{
    if (*sz == '\0' || *sz == ';')
        fprintf(ctxt_output(), "Skip: %d\n", ctxt_getcontext()->skip);
    else
        ctxt_getcontext()->skip = ctxt_atoi(sz, "skip", 0, E_INVSKIP);
}

void ctxt_setibase(const char *sz)
{
    int             n;

    if ((n = ctxt_atoi(sz, "ibase", 0, E_INVSIZE)) == 0)
        fprintf(ctxt_output(), "Input Base: %d\n",
                ctxt_getcontext()->ibase);
    else
    {
        if (n == 1 || n > 36)
            n = 0;
        ctxt_getcontext()->ibase = n;
    }
}

void ctxt_settransize(const char *sz)
{
    int             n;

    if ((n = ctxt_atoi(sz, "transize", 0, E_INVSIZE)) == 0)
        fprintf(ctxt_output(), "Transaction size: %d\n",
                ctxt_getcontext()->tsize);
    else
        ctxt_getcontext()->tsize = n;
}

void ctxt_settypes(const char *op)
{
    ctxt_stack(&ctxt_getcontext()->types, ctxt_opcon(op), "Types");
}

void ctxt_setverbose(const char *op)
{
    ctxt_stack(&ctxt_getcontext()->verbose, ctxt_opcon(op), "Verbose");
}

void ctxt_setxmlrecordtag(const char *tag)
{
    Context        *cp;

    cp = ctxt_getcontext();
    FREE(cp->xmlrecordtag);
    cp->xmlrecordtag = STRDUP(tag);
}

void ctxt_setxmlrecsettag(const char *tag)
{
    Context        *cp;

    cp = ctxt_getcontext();
    FREE(cp->xmlrecsettag);
    cp->xmlrecsettag = STRDUP(tag);
}

void ctxt_setxmlheadertag(const char *tag)
{
    Context        *cp;

    cp = ctxt_getcontext();
    FREE(cp->xmlheadertag);
    cp->xmlheadertag = STRDUP(tag);
}

/*********************\
** CONTEXT ENQUIRIES **
\*********************/

size_t ctxt_getlinenum(void)
{
    return(ctxt_getcontext()->linenum);
}

char *ctxt_geteor(void)
{
    return(ctxt_getcontext()->eor);
}

char *ctxt_getblobdir(void)
{
    return(ctxt_getcontext()->blobdir);
}

int ctxt_getcontinue(void)
{
    return(ctxt_peep(&ctxt_getcontext()->nonstop));
}

char *ctxt_getdate(void)
{
    return(ctxt_getcontext()->date);
}

int ctxt_getdelim(void)
{
    return(ctxt_getcontext()->delim);
}

int ctxt_getescape(void)
{
    return(ctxt_getcontext()->escape);
}

int ctxt_getformat(void)
{
    return(ctxt_peep(&ctxt_getcontext()->format));
}

int ctxt_getheading(void)
{
    return(ctxt_peep(&ctxt_getcontext()->heading));
}

int ctxt_gethexmode(void)
{
    return(ctxt_peep(&ctxt_getcontext()->hexmode));
}

int ctxt_gethistory(void)
{
    return(ctxt_peep(&ctxt_getcontext()->history));
}

int ctxt_getquote(void)
{
    return(ctxt_getcontext()->quote);
}

int ctxt_getsilence(void)
{
    return(ctxt_peep(&ctxt_getcontext()->silence));
}

int ctxt_getsingle(void)
{
    return(ctxt_peep(&ctxt_getcontext()->single));
}

int ctxt_getbmark(void)
{
    return(ctxt_peep(&ctxt_getcontext()->bmark));
}

int ctxt_gettrace(void)
{
    return(ctxt_peep(&ctxt_getcontext()->trace));
}

int ctxt_getskipsize(void)
{
    return(ctxt_getcontext()->skip);
}

int ctxt_getqlimit(void)
{
    return(ctxt_getcontext()->qlimit);
}

int ctxt_getibase(void)
{
    return(ctxt_getcontext()->ibase);
}

int ctxt_gettransize(void)
{
    return(ctxt_getcontext()->tsize);
}

int ctxt_gettypes(void)
{
    return(ctxt_peep(&ctxt_getcontext()->types));
}

int ctxt_getverbose(void)
{
    return(ctxt_peep(&ctxt_getcontext()->verbose));
}

char *ctxt_getxmlrecordtag(void)
{
    return(ctxt_getcontext()->xmlrecordtag);
}

char *ctxt_getxmlrecsettag(void)
{
    return(ctxt_getcontext()->xmlrecsettag);
}

char *ctxt_getxmlheadertag(void)
{
    return(ctxt_getcontext()->xmlheadertag);
}

static void ctxt_printstack(FILE *fp, const char *label, Stack const *s)
{
    int             i;

    fprintf(fp, "%-10s <TOS>", label);
    for (i = s->sp; i >= 0; i--)
    {
        if (i < 0 || i >= DIM(s->stack))
            err_error("context stack - index trouble: stack index = %d\n", i);
        if (s->stack[i] >= DIM(op_list))
            err_error("context stack - oplist trouble: oplist index = %d\n", s->stack[i]);
        fprintf(fp, " %-3s", op_list[s->stack[i]]);
    }
    fprintf(fp, " <BOS>\n");
}

void ctxt_print(FILE *fp)
{
    Context *cp = ctxt_getcontext();

    fprintf(fp, "Level:   %d\n", ctxt_num);
    fprintf(fp, "Input:   %s\n", cp->infile);
    fprintf(fp, "Output:  %s\n", cp->outfile);
    fprintf(fp, "Error:   %s\n", cp->errfile);
    fprintf(fp, "Date:    %s\n", cp->date);
    ctxt_print_str("Delimiter:", sizeof("Delimiter:")-1, &cp->delim,  sizeof(cp->delim));
    ctxt_print_str("Escape:",    sizeof("Delimiter:")-1, &cp->escape, sizeof(cp->escape));
    ctxt_print_str("Quote:",     sizeof("Delimiter:")-1, &cp->quote,  sizeof(cp->quote));
    ctxt_print_str("EOR:",       sizeof("Delimiter:")-1, cp->eor,     strlen(cp->eor));
    if (hist_chkinit())
        fprintf(fp, "History Size:       %d\n", hist_getsize());
    fprintf(fp, "Query Limit:        %d\n", cp->qlimit);
    fprintf(fp, "Transaction Size:   %d\n", cp->tsize);
    fprintf(fp, "Input Base:         %d\n", cp->ibase);
    fprintf(fp, "Blob Directory:     %s\n", cp->blobdir);
    fprintf(fp, "XML Record Set Tag: %s\n", cp->xmlrecsettag);
    fprintf(fp, "XML Record Tag:     %s\n", cp->xmlrecordtag);
    fprintf(fp, "XML Header Tag:     %s\n", cp->xmlheadertag);
    ctxt_printstack(fp, "Format:",    &cp->format);
    ctxt_printstack(fp, "Heading:",   &cp->heading);
    ctxt_printstack(fp, "History:",   &cp->history);
    ctxt_printstack(fp, "Continue:",  &cp->nonstop);
    ctxt_printstack(fp, "Silence:",   &cp->silence);
    ctxt_printstack(fp, "Single:",    &cp->single);
    ctxt_printstack(fp, "Trace:",     &cp->trace);
    ctxt_printstack(fp, "Types:",     &cp->types);
    ctxt_printstack(fp, "Verbosity:", &cp->verbose);
    ctxt_printstack(fp, "Benchmark:", &cp->bmark);
    ctxt_printstack(fp, "Hex Mode:",  &cp->hexmode);
    fflush(fp);
}
