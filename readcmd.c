/*
@(#)File:           $RCSfile: readcmd.c,v $
@(#)Version:        $Revision: 2015.2 $
@(#)Last changed:   $Date: 2015/07/18 03:55:14 $
@(#)Purpose:        Read Commands for SQLCMD
@(#)Author:         J Leffler
@(#)Copyright:      (C) JLSS 1992-93,1995-97,2000-01,2003-05,2007-08,2011,2013,2015
@(#)Product:        SQLCMD Version 90.02 (2016-07-28)
*/

/*TABSTOP=4*/

/*
** Beware: this is some of the more complex code in SQLCMD.  It has to
** deal with input from terminals (using the GNU readline library if it
** is available) and with input from files (avoiding the GNU readline
** library).  It has to deal with prompting.  And it is trivially easy
** to break -- witness SQLCMD 56!
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include "posixver.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "readcmd.h"    /* cmd_read(), cmd_add_to_readline_history(), cmd_set_promptnum() */
#include "chktty.h"     /* chk_tty() */
#include "tokencmp.h"   /* tokencmp() */
#include "sqltoken.h"   /* iustoken() */
#include "kludge.h"
#include "esnprintf.h"
#include "gnureadline.h"

#ifdef HAVE_READLINE_READLINE_H
static int jlss_getc(FILE *fp);
static int jlss_ungetc(int c, FILE *fp);
#define GETC(a)      jlss_getc(a)
#define UNGETC(a,b)  jlss_ungetc(a, b)
#else
#define GETC(a)      getc(a)
#define UNGETC(a,b)  ungetc(a, b)
#endif /* HAVE_READLINE_READLINE_H */

#define CONST_CAST(t,v) ((t)(v))    /* Simulate C++ const_cast<t>(v) */

#define PS1         "SQL[%d]: "
#define LCURLY      '{'
#define RCURLY      '}'

enum { TTY_UNCHECKED = 0, IS_TTY = 1, NO_TTY = 2 };
static FILE    *ttyfp = 0;
static int      ttyok = TTY_UNCHECKED;

static FILE    *ifp;
static char    *src;
static char    *dst;
static char    *end;
static int      tty;
static const char *ps1 = PS1;
static long     cmdnum = 0;

#ifdef HAVE_READLINE_READLINE_H
static int      ubuffer;       /* The gnurl_ungetc buffer. */
static char     promptb[20];
#endif /* HAVE_READLINE_READLINE_H */

static size_t   linenum = 0;    /* Number of line being read - increments when '\n' consumed */

#ifndef lint
/* Prevent over-aggressive optimizers from eliminating ID string */
extern const char jlss_id_readcmd_c[];
const char jlss_id_readcmd_c[] = "@(#)$Id: readcmd.c,v 2015.2 2015/07/18 03:55:14 jleffler Exp $";
#endif /* lint */

void hit_return_to_continue(void)
{
    int c;

    if (ttyok == TTY_UNCHECKED)
    {
        if ((ttyfp = fopen("/dev/tty", "r+")) == 0)
            ttyok = NO_TTY;
        else
            ttyok = IS_TTY;
    }
    if (ttyok == NO_TTY)
        return;
    fprintf(ttyfp, "Hit return to continue: ");
    fflush(ttyfp);
    while ((c = getc(ttyfp)) != EOF && c != '\n')
        ;
    fflush(ttyfp);
}

size_t cmd_get_linenum(void)
{
    return linenum;
}

void cmd_set_linenum(size_t newnum)
{
    linenum = newnum;
}

static int cmd_getc(FILE *fp)
{
    int c = GETC(fp);
    if (c == '\n')
        linenum++;
    return(c);
}

static void cmd_ungetc(char c, FILE *fp)
{
    UNGETC(c, fp);
    if (c == '\n')
        linenum--;
}

/* JL 2006-06-23: How grubby can you get? How slow? */
#undef GETC
#undef UNGETC
#define GETC(c)         cmd_getc(c)
#define UNGETC(c,fp)    cmd_ungetc(c,fp)

#ifdef HAVE_READLINE_READLINE_H

/*
** With GNU readline library available, we use the GNU readline code
** when the input is coming from a terminal, but the regular code when
** it is coming from a file.  We do this by calling the jlss_getc() and
** jlss_ungetc() routines -- via the GETC and UNGETC macros -- and those
** routines switch behaviour based on the file static variable tty which
** gets set by cmd_read(), the main external entry point for this code.
**
** The gnurl_getc() and gnurl_ungetc() functions actually use the GNU
** readline code.
**
** JL 2006-06-27: There's some black magic that needs to be documented.
** Especially if we keep line numbering...
*/

static int gnurl_getc(FILE *stream)
{
    static char *line = NULL;
    static int  pos = 0;
    int         t;
    FEATURE("GNU Readline Enabled");

    rl_instream = stream;       /* Ensure we read the right file. */

    if (ubuffer)                /* First check if we called gnurl_ungetc. */
    {
        t = ubuffer;
        ubuffer = 0;
        return t;
    }

    if (line)
    {
        if (line[pos] != '\0')
            return line[pos++];
        free(line);
        line = NULL;
        if (pos)
        {
            pos = 0;
            return '\n';
        }
    }

    line = readline(promptb);
    pos = 0;
    if (line)
    {
        if (line[0])
            return line[pos++];
        else
            return '\n';
    }
    return EOF;
}

static int gnurl_ungetc(int c, FILE *stream)
{
    ubuffer = c;
    return c;
}

static int jlss_getc(FILE *fp)
{
    if (tty)
        return(gnurl_getc(fp));
    else
        return(getc(fp));
}

static int jlss_ungetc(int c, FILE *fp)
{
    if (tty)
        return(gnurl_ungetc(c, fp));
    else
        return(ungetc(c, fp));
}

#endif /* HAVE_READLINE_READLINE_H */

/* Look at next character in input without removing it from the input stream */
static int      peek(void)
{
    int             c;

    c = GETC(ifp);
    if (c != EOF)
        UNGETC(c, ifp);
    return(c);
}

/* Conditionally store character (in command buffer) */
/* -- no spaces at start of string */
static void     cput(char c)
{
    if (dst < end && (!isspace((unsigned char)c) || dst > src))
        *dst++ = c;
}

/*
** Prompt user if appropriate.
**
** If GNU readline is not available, then print the prompt when the
** input comes from a terminal and otherwise print nothing.  If GNU
** readline is available, the function is never called.  The call to
** fflush(stdout) is not absolutely necessary, but does no harm.
*/

#ifndef HAVE_READLINE_READLINE_H
static void     prompt(char *s)
{
    if (tty)
        fputs(s, stdout);
    fflush(stdout);
}
#endif /* !HAVE_READLINE_READLINE_H */

/* Use primary (PS1) prompt */
static void     prompt_ps1(void)
{
#ifdef HAVE_READLINE_READLINE_H
    if (tty)
    {
        esnprintf(promptb, sizeof(promptb), ps1, cmdnum);
    }
#else
    char            buffer[20];
    esnprintf(buffer, sizeof(buffer), ps1, cmdnum);
    prompt(buffer);
#endif /* HAVE_READLINE_READLINE_H */
}

static void set_ps2(char *buffer, size_t bufsiz)
{
    size_t len;
    esnprintf(buffer, bufsiz, ps1, cmdnum);
    len = strlen(buffer);
    memset(buffer, ' ', len);
    buffer[len - 2] = '>';
}

/* Use secondary (PS2) prompt */
static void     prompt_ps2(void)
{
#ifdef HAVE_READLINE_READLINE_H
    if (tty)
        set_ps2(promptb, sizeof(promptb));
#else
    char            buffer[20];
    set_ps2(buffer, sizeof(buffer));
    prompt(buffer);
#endif /* HAVE_READLINE_READLINE_H */
}

/* Read quoted string */
/* NB: in SQL, backslash is not significant! */
/* Further, '''' is a valid string containing a single quote */
/* However, it is parsed by readquote() as '' followed by '' */
static void readquote(int c0)
{
    int c;

    cput(c0);
    while ((c = GETC(ifp)) != c0)
    {
        if (c == EOF)
            return;
        cput(c);
        if (c == '\n')
            prompt_ps2();
    }
    cput(c0);
}

/* Skip over blanks and tabs */
static int      readblanks(void)
{
    int             c;

    while ((c = GETC(ifp)) != EOF)
    {
        if (!isspace(c) || c == '\n')
            break;
    }
    if (c != EOF && c != '\n')
        UNGETC(c, ifp);
    return(c);
}

/* Record the command number */
void            cmd_set_promptnum(long num)
{
    if (num >= 0)
        cmdnum = num + 1;
}

/* Read a comment between braces '{}' (opening brace already read) */
static void     readbrace(void)
{
    int             c;

    cput(LCURLY);
    while ((c = GETC(ifp)) != RCURLY)
    {
        if (c == EOF)
            return;
        cput((char)c);
        if (c == '\n')
            prompt_ps2();
    }
    cput(RCURLY);
}

/* Read a hash or double-dash comment (up to end of line) */
static void     readeolcomment(void)
{
    int             c;

    while ((c = GETC(ifp)) != '\n')
    {
        if (c == EOF)
            return;
        cput((char)c);
    }
    cput('\n');
}

/* Read a bang (!) or at (@) command (up to end of line) */
static void     readbang(void)
{
    int             c;

    c = readblanks();
    while ((c = GETC(ifp)) != '\n')
    {
        if (c == EOF)
            return;
        cput((char)c);
    }
}

/*
** Read a command up to the first un-quoted semi-colon
** Except for CREATE PROCEDURE/FUNCTION, this will be an SQL command.
** NB: w.e.f 2001-03-03, no longer recognizes # as marking a comment because #
** is used in IUS 9.2x to mark the type of a statement local variable (SLV or
** OUT parameter).
*/
static void     readsemicolon(void)
{
    char             c;
    int     io_c;

    while ((io_c = GETC(ifp)) != EOF)
    {
        c = io_c;
        if (c == '"' || c == '\'')
            readquote(c);
        else if ((c == '#' && peek() == '!') || (c == '-' && peek() == '-'))
        {
            /* Translate #! comments to --#! comments */
            if (c == '#')
            {
                cput('-');
                cput('-');
            }
            cput(c);
            readeolcomment();
            prompt_ps2();
        }
        else if (c == '{')
            readbrace();
        else if (c == '\n')
        {
            if (dst == src)
                break;
            cput(c);
            prompt_ps2();
        }
        else if (c == ';')
        {
            if (dst != src)
                cput(c);
            /* JL 2003-06-04: This call to readblanks() is needed, but
            ** the return value is not.  It prevents double-prompting
            ** and double-spacing because it eats up a newline.
            */
            c = readblanks();
            break;
        }
        else
            cput(c);
    }
    *dst = '\0';
}

/*
** Does this string contain END PROCEDURE/FUNCTION as two adjacent words?
** Function is_end_proc() is no longer confused by BEGIN <block> END END
** PROCEDURE (which is valid syntax), thanks to Mahesh Chandrasekhar
** <Mahesh.Chandrasekhar@sciatl.com>, who reported this bug on 2001-07-11.
** Also upgraded to require END FUNCTION with CREATE FUNCTION and END
** PROCEDURE with CREATE PROCEDURE, and to recognize EXTERNAL NAME as
** indicating that this is (or should be) a complete statement for
** registering an externally defined function (which does not need END
** PROCEDURE).
*/
static int is_end_proc(const char *s, const char *keyword)
{
    char *t;
    const char *p = s;
    static const char kw_end[] = "end";
    static const char kw_external[] = "external";
    static const char kw_name[] = "name";

    while (*(t = iustoken(p, &p)) != '\0')
    {
        int c1;
        while ((c1 = tokencmp(t, p - t, kw_end, sizeof(kw_end)-1)) == 0 ||
               tokencmp(t, p - t, kw_external, sizeof(kw_external)-1) == 0)
        {
            t = iustoken(p, &p);
            /* If first word is END -- look for keyword (FUNCTION or PROCEDURE) */
            /* If first word is EXTERNAL -- look for NAME */
            if ((c1 == 0 && tokencmp(t, p - t, keyword, strlen(keyword)) == 0) ||
                (c1 != 0 && tokencmp(t, p - t, kw_name, sizeof(kw_name)-1) == 0))
            {
                /* Second word matches keyword or NAME */
                return(1);
            }
        }
    }
    return(0);
}

/* Skip over a quoted string. */
/* NB: in SQL, backslash is not significant! */
static const char *skipquote(const char *s)
{
    char            c0;         /* Quote character */
    char            c1;         /* Previous character */
    char            c2;         /* Current character */

    for (c0 = c1 = *s++; (c2 = *s) != '\0'; s++, c1 = c2)
    {
        if (c2 == c0)
            break;
    }
    return(s);
}

/*
** Does the CREATE PROCEDURE statement require a body?
** This one doesn't!
** CREATE PROCEDURE xyz(i INTEGER, j REFERENCES TEXT, k CHAR(30))
** END PROCEDURE DOCUMENT "empty procedure" WITH LISTING IN "xx";
** Nor does this one:
** CREATE PROCEDURE xyz(i INTEGER, j REFERENCES TEXT, k CHAR(30))
** SPECIFIC "myuser".somename
** END PROCEDURE DOCUMENT "empty procedure" WITH LISTING IN "xx";
** Nor does this one:
** CREATE FUNCTION Equal(longlong,longlong) RETURNS BOOLEAN WITH
** (PARALLELIZABLE) EXTERNAL NAME '$USERFUNCDIR/longlong.udr(ll_equal)'
** LANGUAGE C NOT VARIANT;
** In this latter variant, the EXTERNAL NAME keywords are mandatory.
** External procedures cannot have a RETURNS clause.  You can have a
** specific name before the WITH clause.  You can't have a semi-colon in
** that code; you can have an END FUNCTION (or END PROCEDURE) in it.
**
** Argument is a pointer to the character (blank?) after CREATE
** PROCEDURE or CREATE FUNCTION (possibly with DBA in there).  The
** statement ends at a semi-colon or EOF.
** Returns TRUE (1) if END PROCEDURE not detected in statement.
** Required processing:
** Find open parenthesis (syntax error if none).
** Find balanced close parenthesis to allow for char(30) parameter types
** (syntax error if none).
** Determine whether next keywords are END PROCEDURE.  Valid alternatives
** include RETURNING and DEFINE and almost any other statement introducer.
*/
static const char *needs_proc_body(const char *s, const char *keyword)
{
    char        c;
    int         bc = 0;     /* Bracket count */
    int         bl = 0;     /* Bracket level */
    const char *p;

    for (p = s; (c = *p) != '\0'; p++)
    {
        if (c == '(')
        {
            bc++;
            bl++;
        }
        else if (c == ')')
        {
            bc++;
            bl--;
            if (bl == 0)
            {
                p++;
                break;
            }
        }
        else if (c == '\'' || c == '"')
        {
            /* Must worry about quoted strings for defaults! */
            p = skipquote(p);
        }
    }
    if (bc > 0 && bl == 0 && c != '\0')
    {
        /* Found balanced brackets OK.  Does END PROCEDURE follow? */
        const char *rv = is_end_proc(p, keyword) ? 0 : keyword;
        return rv;
    }
    /* Syntactically incorrect, so does not need procedure body */
    return(0);
}

/*
** Is this a CREATE PROCEDURE or CREATE DBA PROCEDURE statement?
** This is not necessarily the cleanest way to handle it, but it
** works.
** NB: we have to handle deviant procedures such as:
** CREATE PROCEDURE xyz(i integer, j references text, k char(30))
** END PROCEDURE DOCUMENT "empty procedure" WITH LISTING IN "xx";
** Function needs_proc_body() tells us whether we need to read a procedure
** body (meaning more semi-colon delimited statements) or not.
*/
static const char *is_crt_proc(const char *s)
{
    const char *t;
    static const char kw_create[] = "create";
    static const char kw_dba[] = "dba";
    static const char kw_procedure[] = "procedure";
    static const char kw_function[] = "function";

    t = iustoken(s, &s);
    if (s == t || tokencmp(t, s - t, kw_create, sizeof(kw_create)-1) != 0)
    {
        /* String is empty or first word is not CREATE */
        return(0);
    }
    t = iustoken(s, &s);
    if (tokencmp(t, s - t, kw_dba, sizeof(kw_dba)-1) == 0)
    {
        /* Skip DBA in CREATE DBA PROCEDURE */
        t = iustoken(s, &s);
    }
    if (tokencmp(t, s - t, kw_procedure, sizeof(kw_procedure)-1) == 0)
    {
        const char *rv = needs_proc_body(s, kw_procedure);  /* Word is PROCEDURE */
        return rv;  /* Word is PROCEDURE */
    }
    else if (tokencmp(t, s - t, kw_function, sizeof(kw_function)-1) == 0)
    {
        const char *rv = needs_proc_body(s, kw_function);   /* Word is FUNCTION */
        return rv;  /* Word is FUNCTION */
    }
    return(0);
}

/* Read an SQL command, allowing for CREATE PROCEDURE statements. */
/* Most SQL commands are terminated by an unquoted semi-colon.    */
/* CREATE PROCEDURE statements are a pain, because you usually    */
/* have to read more than one semi-colon's worth of text to get   */
/* the complete statement.  But there is an exception: an empty   */
/* procedure requires just a single semi-colon, as illustrated by */
/* CREATE PROCEDURE xyz(i integer, j references text, k char(30)) */
/* END PROCEDURE DOCUMENT "empty procedure" WITH LISTING IN "xx"; */
/* The designer (if that is the correct word for him/her) of SPL  */
/* should be strung up, hung, drawn, quartered, resuscitated, and */
/* the whole process reiterated until he/she/it agrees to repent, */
/* recant, undo the damage already done (requires a time machine, */
/* but apart from that, it shouldn't be too hard!), and redesigns */
/* SPL so it does not use semi-colons except at the end of the    */
/* procedure, and puts the DOCUMENT and WITH LISTING IN clauses   */
/* before the keywords END PROCEDURE.  Oh well, one can dream!    */
static void     readcmd(void)
{
    int         crt_proc = 0;
    char       *mark = src;
    const char *keyword = 0;

    while (1)
    {
        readsemicolon();
        if (crt_proc == 0 && (keyword = is_crt_proc(src)) != 0)
            crt_proc = 1;
        else if (crt_proc == 0 || is_end_proc(mark, keyword))
            break;
        else if (peek() == EOF)
            break;
        cput('\n');
        mark = dst;
        prompt_ps2();
    }
}

/*
** Read a command.  Allow for SQL commands, shell escapes (! commands) and
** internal commands (@ commands).
*/
int             cmd_read(FILE *fp, char *buffer, int length)
{
    int             c;

    /* Record whether input is a tty or not */
    tty = chk_tty(fp);

    ifp = fp;
    src = buffer;
    end = buffer + length - 1;
    dst = src;
    do
    {
        prompt_ps1();
    } while ((c = readblanks()) == '\n');
    if (c == EOF && tty)
    {
        fputs("\n", stdout);
        fflush(stdout);
    }
    else if (c == '@' || c == '!')
        readbang();
    else
        readcmd();
    *dst = '\0';
    return((c == EOF) ? EOF : 0);
}

void     cmd_add_to_readline_history(const char *cmd)
{
#ifdef HAVE_READLINE_READLINE_H
    if (cmd && *cmd)
        add_history(CONST_CAST(char *, cmd));
#endif /* HAVE_READLINE_READLINE_H */
}

#ifdef TEST

#include <unistd.h>
#include "stderr.h"

#ifndef BUFFERSIZE
#define BUFFERSIZE  64
#endif /* BUFFERSIZE */

#ifndef MAX_BUFFERSIZE
#define MAX_BUFFERSIZE (256*1024)
#endif /* MAX_BUFFERSIZE */

static const char usestr[] = "[-B buffsize][-V]";

int main(int argc, char **argv)
{
    char    buffer[BUFFERSIZE];
    long    promptno;
    char   *bufptr = buffer;
    size_t  buflen = sizeof(buffer);
    int     opt;

    err_setarg0(argv[0]);

    while ((opt = getopt(argc, argv, "B:V")) != -1)
    {
        switch (opt)
        {
        case 'B':
            buflen = strtoul(optarg, 0, 0);
            if (buflen == 0)
                err_error("buffer length zero (or non-numeric) - %s\n", optarg);
            else if (buflen > MAX_BUFFERSIZE)
                err_error("buffer length %s too big - maximum is %lu\n", optarg, MAX_BUFFERSIZE);
            else if ((bufptr = (char *)malloc(buflen)) == 0)
                err_error("out of memory\n");
            break;
        case 'V':
            err_version("READCMD", &"@(#)$Revision: 2015.2 $ ($Date: 2015/07/18 03:55:14 $)"[4]);
            break;
        default:
            err_usage(usestr);
            break;
        }
    }

    if (optind != argc)
    {
        err_remark("unexpected extra arguments - starting with '%s'\n", argv[optind]);
        err_usage(usestr);
    }

    printf("Buffer size: %lu\n", (unsigned long)buflen);

    promptno = 0;
    while (cmd_read(stdin, bufptr, buflen) != EOF)
    {
        if (strlen(bufptr) > 0)
        {
            cmd_set_promptnum(promptno++);
            printf("CMD (line %d): %s\n", cmd_get_linenum(), bufptr);
        }
    }
    puts("EOF:");
    return(0);
}

#endif /* TEST */
