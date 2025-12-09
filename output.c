/*
@(#)File:           $RCSfile: output.c,v $
@(#)Version:        $Revision: 2015.1 $
@(#)Last changed:   $Date: 2015/11/09 01:37:12 $
@(#)Purpose:        Print data from SQLDA structure
@(#)Author:         J Leffler
@(#)Copyright:      (C) JLSS 1992-93,1995,1997-2005,2007-08,2010-13,2015
@(#)Product:        SQLCMD Version 90.02 (2016-07-28)
*/

/*TABSTOP=4*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#else
extern int read(int fd, char *buffer, int len);
extern int close(int fd);
extern long lseek(int fd, long off, int rel);
#endif /* HAVE_UNISTD_H */

#include "context.h"
#include "emalloc.h"
#include "esqlc.h"
#include "describe.h"
#include "output.h"
#include "sqlcmd.h"
#include "sqlerr.h"
#include "jlsstools.h"
#include "esnprintf.h"
#include "internal.h"
#include "ixblob.h"
#include "decsci.h"
#include "esqlutil.h"
#include "kludge.h"
#include "usesqlda.h"

/* -- Constant Definitions -- */

/*
** Compile with -DDEBUGGING_OUTPUT_C=1 only in dire straits
*/
#ifndef DEBUGGING_OUTPUT_C
#define DEBUGGING_OUTPUT_C 0
#endif /* DEBUGGING_OUTPUT_C */

enum
{
    debugging_output_c = DEBUGGING_OUTPUT_C
};

/* Size of string to receive ASCII version of decimal/datetime/interval */
enum
{
    BUFFERSIZE = 40
};

enum
{
    QUOTE_NONE = 0x0,
    QUOTE_LEAD = 0x1,
    QUOTE_TAIL = 0x2,
    QUOTE_BOTH = (QUOTE_LEAD | QUOTE_TAIL)
};

enum
{
    NBSP = 0xA0
}; /* Non-breaking space in ISO 8859-x and Unicode code sets */

enum
{
    FMT_DOUBLE = 0,
    FMT_FLOAT = 1,
    FMT_SHORT = 2,
    FMT_INTEGER = 3,
    FMT_BIGINT = 4

};

/* -- Declarations -- */

enum
{
    F_VARIABLE = 0,
    F_FIXED = 1
};

static const char hexdigit[] = "0123456789ABCDEF";
static int hexmode;
static char delim;
static char escap;
static char quote;
static char *eor;
static int formt;
static int f_fixvar; /* 1 if format is fixed; 0 if variable */
static FILE *ofile;
static char fixwn; /* Printing blob in fixed mode warning */
static const char *const *fmt;
/* INDENT OFF */
/* With luck, using PRId_ixInt[24] gives some protection on 64-bit platforms */
static const char *const fmt_v[] = {"%-.14G", "%-.7G", "%" PRId_ixInt2, "%" PRId_ixInt4, "%" PRId_ixInt8};
static const char *const fmt_f[] = {"%21.14E", "%14.7E", "%6" PRId_ixInt2, "%11" PRId_ixInt4, "%1" PRId_ixInt8};
/* INDENT ON */

static const char f_xmlheading_fmt[] =
    " <column number=\"%d\" name=\"%s\" type_number=\"%d\" type_length=\"%d\" type_name=\"%s\"/>%s";

static const char f_xmlidstring[] = "<?xml version='1.0'?>";

static const char html_td[] = "td";
static const char html_tr[] = "tr";
static const char html_th[] = "th";
static const char html_tb[] = "table border=\"1\"";
static const char html_nb[] = "&nbsp;";

static int exp_count = 0; /* Expression counter */
static short ind_notnull = 0;

#ifndef lint
/* Prevent over-aggressive optimizers from eliminating ID string */
extern const char jlss_id_output_c[];
const char jlss_id_output_c[] = "@(#)$Id: output.c,v 2015.1 2015/11/09 01:37:12 jleffler Exp $";
#endif /* lint */

static void reset_column_exp_count(void)
{
    exp_count = 0;
}

static const char *column_name(Sqlva *sqlv)
{
    const char *rv;
    static char buffer[16]; /* Big enough for sqlcmd_999 with space to spare */

    if (sqlv->sqlname[0] == '(' &&
        (strcmp(sqlv->sqlname, "(expression)") == 0 ||
         strcmp(sqlv->sqlname, "(constant)") == 0))
    {
        esnprintf(buffer, sizeof(buffer), "sqlcmd_%03d", ++exp_count);
        rv = buffer;
    }
    else
        rv = sqlv->sqlname;
    return (rv);
}

static int io_error(FILE *fp)
{
    int rc = 0;

    if (ferror(fp) != 0)
    {
        error_746("File I/O error");
        rc = -1;
    }
    return (rc);
}

#ifdef DEBUG
#include "stderr.h"
static void dump_print_mode(void)
{
    if (getenv("SQLCMD_DEBUG_OUTPUT") != 0)
    {
        err_logmsg(ctxt_error(), ERR_REM, 1,
                   "HexMode = %d, Format = %d, Delim = %d, Quote = %d, Escape = %d, EOR = %d\n",
                   hexmode, formt, delim, quote, escap, eor[0]);
    }
}
#define DUMP_PRINT_MODE() dump_print_mode()
#else
#define DUMP_PRINT_MODE() ((void)0)
#endif /* DEBUG */

/*
** Record current printing mode information.
** This saves a lot of messing around with complex argument
** lists, while not using unreasonably many global variables.
*/
static void set_printmode(void)
{
    hexmode = ctxt_gethexmode();
    formt = ctxt_getformat();
    delim = ctxt_getdelim();
    quote = ctxt_getquote();
    escap = ctxt_getescape();
    ofile = ctxt_output();
    eor = ctxt_geteor();
    if (formt == OP_FIXED || formt == OP_FIXSEP || formt == OP_FIXDEL)
    {
        f_fixvar = F_FIXED;
        fmt = fmt_f;
    }
    else
    {
        f_fixvar = F_VARIABLE;
        fmt = fmt_v;
    }
    fixwn = 0;
    DUMP_PRINT_MODE();
}

static int xml_output(FILE *fp, char c)
{
    int rc;

    if (c == '<')
        rc = fputs("&lt;", fp);
    else if (c == '>')
        rc = fputs("&gt;", fp);
    else if (c == '&')
        rc = fputs("&amp;", fp);
    else if ((unsigned char)c == NBSP)
        rc = fputs(html_nb, fp);
    else
        rc = putc(c, fp);
    return rc;
}

/*
** Print one field, escaping embedded delimiters.
** NB: it does not print the delimiter at the end of the field, though
**     it does need to know what the delimiter is in order to escape it.
** NB: it does not strip trailing blanks.
** qflag & QUOTE_LEAD => print leading quote
** qflag & QUOTE_TAIL => print trailing quote
*/
static void outtext(const char *str, int len, int qflag)
{
    const char *s = str;
    const char *e = str + len;
    char e1 = escap;
    char e2 = escap;

    /* DUMP_PRINT_MODE(); // Emergency only */
    if (formt == OP_QUOTE && (qflag & QUOTE_LEAD))
    {
        e1 = quote;
        if (putc(e1, ofile) == EOF)
            return;
    }
    if (formt == OP_MARKDOWN)
    {
        while (s < e)
        {
            unsigned char c = *s++;
            if (c == '\r' && *s == '\n')
            {
                if (fprintf(ofile, "<br>") == EOF)
                {
                    return;
                }
                s++;
            }
            else if (c == '\r' || c == '\n')
            {
                if (fprintf(ofile, "<br>") == EOF)
                    return;
            }
            else if (c == '|')
            {
                if (fprintf(ofile, "\\|") == EOF)
                {
                    return;
                }
            }
            else if (c == '\\' && *s == '\\')
            {
                if (fprintf(ofile, "\\\\") == EOF)
                    return;
                s++;
            }
            else if (c == '\\')
            {
                if (fprintf(ofile, "\\\\") == EOF)
                    return;
            }
            else if ( isprint(c) ) {
                if (putc(c, ofile) == EOF)
                    return;
            }
        }
    }
    else if (formt == OP_XML || formt == OP_HTML)
    {
        while (s < e)
        {
            unsigned char c = *s++;
            if (xml_output(ofile, c) == EOF)
                return;
        }
    }
    else if (hexmode == OP_ON)
    {
        while (s < e)
        {
            unsigned char c = *s++;

            if (iscntrl(c))
            {
                /* DB-Access -X outputs backslash and 2 hex digits for 0x01..0x1F and 0x7f for UNLOAD (only) */
                if (fprintf(ofile, "\\%.2X", c) != 3)
                    return;
            }
            else
            {
                if (putc(c, ofile) == EOF)
                    return;
            }
        }
    }
    else if (formt == OP_QUOTE)
    {
        while (s < e)
        {
            unsigned char c = *s++;
            /* CSV: double up quotes; for SQLCMD, escape newlines (MS Excel doesn't do that). */
            /* TristanJBall@sportsbet: */
            /* ... Removed, the fact that excel doesn't do it is a strong arguement for us not doing it either! */
            /* End tjb@sb */
            /* Delimiters (commas) and escapes (backslashes) don't matter */
            if (((c == e1) && putc(e1, ofile) == EOF))
                return;

            if (putc(c, ofile) == EOF)
                return;
        }
    }
    /* Is revised test for 1st char of EOR sufficient? */
    /* For EOR = CRLF, then need to escape CR but possibly not LF */
    else if (f_fixvar == F_VARIABLE)
    {
        while (s < e)
        {
            unsigned char c = *s++;

            if (c == delim || c == e1 || c == e2 || c == *eor)
            {
                if (putc(e2, ofile) == EOF)
                    return;
            }
            if (putc(c, ofile) == EOF)
                return;
        }
    }

    /*
    while (s < e)
    {
        unsigned char c = *s++;
        if (formt == OP_XML || formt == OP_HTML)
        {
            if (xml_output(ofile, c) == EOF)
                return;
        }
        else if (hexmode == OP_ON && iscntrl(c))
        {
            // DB-Access -X outputs backslash and 2 hex digits for 0x01..0x1F and 0x7f for UNLOAD (only)
            if (fprintf(ofile, "\\%.2X", c) != 3)
                return;
        }
        else
        {
            if (formt == OP_QUOTE)
            {
                // CSV: double up quotes; for SQLCMD, escape newlines (MS Excel doesn't do that).
                // Delimiters (commas) and escapes (backslashes) don't matter
                if (((c == e1) && putc(e1, ofile) == EOF) || ((c == '\n') && putc(e2, ofile) == EOF))
                    return;
            }
            // Is revised test for 1st char of EOR sufficient?
            // For EOR = CRLF, then need to escape CR but possibly not LF
            else if (f_fixvar == F_VARIABLE && (c == delim || c == e1 || c == e2 || c == *eor))
            {
                if (putc(e2, ofile) == EOF)
                    return;
            }
            if (putc(c, ofile) == EOF)
                return;
        }
    }
    */

    if (formt == OP_QUOTE && (qflag & QUOTE_TAIL))
        putc(e1, ofile);
}

/*
** Print one field, escaping embedded delimiters.
** NB: it does not print the delimiter at the end of the field, though
**     it does need to know what the delimiter is in order to escape it.
** qflag & QUOTE_LEAD => print leading quote
** qflag & QUOTE_TAIL => print trailing quote
*/
static void output(const char *str, int qflag)
{
    outtext(str, strlen(str), qflag);
}

/* Print a BYTE blob */
/* Horribly wasteful of memory! */
static void outbyte(char *buffer, long len, int qflag)
{
    char *spc;
    char c;
    char *dst;
    char *src;
    char *end;

    spc = (char *)MALLOC(2 * len + 1);
    jb_register(spc);
    src = buffer;
    dst = spc;
    end = buffer + len;

    while (src < end)
    {
        c = *src++;
        *dst++ = hexdigit[(c >> 4) & 0xF];
        *dst++ = hexdigit[c & 0xF];
    }
    *dst = '\0';
    outtext(spc, dst - spc, qflag);

    jb_unregister(spc);
    FREE(spc);
}

/* Print a complete blob */
static void outblob(Blob *blob)
{
    char buffer[BUFSIZ];
    long n;

#ifdef DEBUG_BLOB_PRINTING
    dump_blob(ctxt_error(), "outblob", blob);
#endif /* DEBUG_BLOB_PRINTING */

    if (f_fixvar == F_FIXED && fixwn == 0)
    {
        cmd_warning(E_BLOBFIXED, "");
        fixwn = 1;
    }

    if (blob->loc_indicator == -1)
        return;

    if (blob->loc_loctype == LOCMEMORY)
    {
        if (blob->loc_buffer != NIL(char *))
        {
            if (blob->loc_type == SQLTEXT)
                outtext(blob->loc_buffer, blob->loc_size, QUOTE_BOTH);
            else
                outbyte(blob->loc_buffer, blob->loc_size, QUOTE_BOTH);
        }
        blob_release(blob, 0);
        blob_locate(blob, blob_getlocmode());
    }
    else if (blob->loc_loctype == LOCFNAME || blob->loc_loctype == LOCFILE)
    {
        /* assert: blob->loc_fd is open for reading */
        outtext("", 0, QUOTE_LEAD);
        lseek(blob->loc_fd, 0L, 0);
        while ((n = read(blob->loc_fd, buffer, sizeof(buffer) - 1)) > 0)
        {
            buffer[n] = '\0';
            if (blob->loc_type == SQLTEXT)
                outtext(buffer, n, QUOTE_NONE);
            else
                outbyte(buffer, n, QUOTE_NONE);
        }
        outtext("", 0, QUOTE_TAIL);
        close(blob->loc_fd);
        blob_release(blob, 0);
        blob_locate(blob, blob_getlocmode());
    }
    else if (blob->loc_loctype == LOCUSER)
        cmd_error(E_INTERNAL, "not coded to handle blob type LOCUSER");
    else
        cmd_error(E_INTERNAL, "unkown blob type");
}

/*
** jlss_stchar - blank pad string s1 into s2; length n includes NUL at end.
** A more civilized interface than stchar.  It relies on there being enough
** space in s2 - and is not separately told how big s2 is!
*/
static void jlss_stchar(const char *s1, char *s2, int n)
{
    stchar(CONST_CAST(char *, s1), s2, n - 1);
    s2[n - 1] = '\0';
}

/*
** jlss_ldchar - copy s1 (length n) to s2, stripping trailing blanks.
** Return pointer to terminating NUL '\0'.
** A more civilised interface than ldchar.
** Preserves embedded NULs in s1.
** Do not modify s1 (unless s1 and s2 are aliases).
** assert(s1 == s2 || (s1 < s2 && s1 + n < s2) || (s2 < s1 && s2 + n < s1));
** NB: s2 must have space for n+1 characters if there are no trailing blanks.
*/
static char *jlss_ldchar(const char *s1, int n, char *s2)
{
    const char *e1 = s1 + n - 1;
    char *e2 = s2;
    const char *src = s1;

    while (e1 > s1 && *e1 == ' ')
        e1--;
    if (*e1 != ' ')
    {
        while (src <= e1)
            *e2++ = *src++;
    }
    *e2 = '\0';
    return (e2);
}

/* rjust() - right justify string s in field of width w */
static char *rjust(char *s, size_t w)
{
    static char buffer[BUFFERSIZE];
    int len;

    assert(w < sizeof(buffer));
    len = strlen(s);
    assert(len <= w);
    memset(buffer, ' ', w - len);
    memmove(buffer + w - len, s, len + 1);
    return (buffer);
}

/* Output a string as a string and a sequence of hex numbers */
/* DEBUGGING tool */
static void dumpbuff(unsigned char *str, size_t len)
{
    size_t i;
    printf("<<%s>>\n", str);
    for (i = 0; i < len; i++)
        printf("%02X ", str[i]);
    putchar('\n');
}

/* Output a string (which is not null) */
/* Chops trailing blanks, but ensure field is not empty */
static void outstr(char *str, int len)
{
    char *end;

    if (debugging_output_c)
        dumpbuff((unsigned char *)str, len + 1);
    if (f_fixvar == F_FIXED)
    {
        jlss_stchar(str, str, len + 1);
        end = str + len;
    }
    else
        end = jlss_ldchar(str, len, str);
    if (str[0] == '\0')
    {
        strcpy(str, " ");
        end = str + 1;
    }
    assert(*end == '\0');
    outtext(str, end - str, QUOTE_BOTH);
}

/* Chops trailing blanks, but ensure field is not empty */
static void outintstr(char *str, int len)
{
    char *end;

    if (debugging_output_c)
        dumpbuff((unsigned char *)str, len + 1);
    if (f_fixvar == F_FIXED)
    {
        jlss_stchar(str, str, len + 1);
        end = str + len;
    }
    else
        end = jlss_ldchar(str, len, str);
    if (str[0] == '\0')
    {
        strcpy(str, " ");
        end = str + 1;
    }
    assert(*end == '\0');
    outtext(str, end - str, QUOTE_NONE);
}

/* Output a fixed-width blank field -- for nulls */
static void blankput(int len)
{
    int i;

    for (i = 0; i < len; i++)
        putc(' ', ofile);
}

static void outvarchar(const Sqlva *col)
{
    /* Preserve trailing blanks on VARCHAR data */
    size_t len = strlen(col->sqldata);
    /*
    ** Deal with DB-Access convention '\ ' indicates
    ** zero-length non-null VARCHAR, NVARCHAR or LVARCHAR.
    */
    if (len == 0 && (formt == OP_SELECT || formt == OP_UNLOAD))
        fputs("\\ ", ofile);
    else if (f_fixvar == F_FIXED)
    {
        outtext(col->sqldata, len, QUOTE_LEAD);
        blankput(col->sqllen - 1 - len);
        outtext("", 0, QUOTE_TAIL);
    }
    else
        outtext(col->sqldata, len, QUOTE_BOTH);
}

#ifdef ESQLC_IUSTYPES
/* Output an LVARCHAR value as a string */
/**
** JL 2003-04-09: The return value from ifx_var_getlen() usually appears
** to include 2 extra characters.  It is not clear why, or when.  This
** (bug?) was previously noted in the context of DBD::Informix.  It is
** also, apparently, not 100% consistent - hence, at least for the time
** being, use strlen() instead of ifx_var_getlen() on the data.  This
** works while we're dealing with LVARCHAR used as a long VARCHAR; it
** won't work if it stores binary data.
** JL 2004-04-04: Also note that ifx_var_getdata() returns a null
** pointer for zero-length non-null values.
*/
static void outlvarchar(const Sqlva *col)
{
    void *lvc = col->sqldata;
    /*size_t len = ifx_var_getlen(&lvc) - 2;*/
    const char *data = (const char *)ifx_var_getdata(&lvc); /*=C++=*/
    size_t len;

    if (data == 0)
        data = "";
    len = strlen(data);

    /*
    ** Deal with DB-Access convention '\ ' indicates
    ** zero-length non-null VARCHAR, NVARCHAR or LVARCHAR.
    */
    if (len == 0 && (formt == OP_SELECT || formt == OP_UNLOAD)) /* F_VARIABLE? */
        fputs("\\ ", ofile);
    else if (f_fixvar == F_FIXED)
    {
        outtext(data, len, QUOTE_LEAD);
        blankput(col->sqllen - 1 - len);
        outtext("", 0, QUOTE_TAIL);
    }
    else
        outtext(data, len, QUOTE_BOTH);
}

/* Output a BOOLEAN value as a string */
static void outboolean(char *boolval)
{
    const char *data = (*boolval == '\0') ? "f" : "t";
    char buffer[2];
    strcpy(buffer, data);
    outstr(buffer, 1);
}

/* right_justify() - convert left justified number into right justified */
/* Convert " 123456......" to "...... 123456" (where the dots are really blanks) */
static void right_justify(char *input, size_t width)
{
    size_t len = byleng(input, width);
    char *end = input + width;
    char *src = input + len;

    assert(input != 0 && width != 0 && len <= width);

    /* Copy numbers right to left to final position */
    while (src > input)
        *--end = *--src;

    /* Blank fill leading vacated spaces */
    end = input + (width - len);
    while (src < end)
        *src++ = ' ';
}

/* Output an INT8 or SERIAL8 value as a string */
static void outint8(ifx_int8_t *i8)
{
    char buffer[32];
    size_t tw = rtypwidth(SQLINT8, 10);
    /**
    ** JL 2003-04-07: Believe it or not, ifx_int8toasc() is defined
    ** not to null terminate the damn string!  See 5423.pdf (ESQL/C
    ** Programmers Reference Manual for IDS 9.21!  (Puke!)
    ** It also blank pads to string length.
    */
    ifx_int8toasc(i8, buffer, sizeof(buffer));
    buffer[tw] = '\0';
    if (f_fixvar == F_FIXED)
        right_justify(buffer, tw);
    outintstr(buffer, strlen(buffer));
}
#endif /* ESQLC_IUSTYPES */

#ifdef ESQLC_BIGINT
/* Output a BIGINT or BIGSERIAL value as a string */
static void outbigint(bigint i8)
{
    char buffer[32];
    /* biginttoasc() does not blank pad and does null terminate.  */
    biginttoasc(i8, buffer, sizeof(buffer), 10);
    if (f_fixvar == F_FIXED)
    {
        size_t len = strlen(buffer);
        size_t wid = rtypwidth(SQLINFXBIGINT, 8);
        while (len < wid)
            buffer[len++] = ' ';
        buffer[len] = '\0';
        right_justify(buffer, wid);
    }
    outintstr(buffer, strlen(buffer));
}
#endif /* ESQLC_BIGINT */

/* Print a null field.  Not hard unless the format is fixed! */
static void print_null(const Sqlva *col)
{
    if (formt == OP_HTML)
    {
        static const char nbsp_str[] = {NBSP, 0};
        outtext(nbsp_str, sizeof(nbsp_str) - 1, QUOTE_NONE);
    }
    else if (f_fixvar == F_VARIABLE)
        outtext("", 0, QUOTE_NONE);
    else
    {
        /* Now what?  Same width as ... */
        int len = 0;
        switch (col->sqltype & ~SQLDISTINCT)
        {
        case CDECIMALTYPE:
        case CMONEYTYPE:
        case SQLDECIMAL:
        case SQLMONEY:
            if (PRECDEC(col->sqllen) == 0xFF)
                len = (PRECTOT(col->sqllen) + 7);
            else
                len = (PRECTOT(col->sqllen) + 1 + (PRECDEC(col->sqllen) > 0));
            break;

        case CFLOATTYPE:
        case SQLSMFLOAT:
            len = 14; /* See fmt_f, and Ugh! */
            break;

        case CDOUBLETYPE:
        case SQLFLOAT:
            len = 21; /* See fmt_f, and Ugh! */
            break;

#ifdef SQLNCHAR
        case SQLNCHAR:
        case SQLNVCHAR:
#endif /* SQLNCHAR */

        case CSTRINGTYPE:
        case CFIXCHARTYPE:
        case SQLCHAR:
        case CCHARTYPE:
        case CVCHARTYPE:
        case SQLVCHAR:
            len = col->sqllen - 1;
            break;

        case CBIGINTTYPE:
        case SQLBIGSERIAL:
        case SQLINFXBIGINT:
        case CINT8TYPE:
        case SQLSERIAL8:
        case SQLINT8:
        case CSHORTTYPE:
        case SQLSMINT:
        case CINTTYPE:
        case CLONGTYPE:
        case SQLINT:
        case SQLSERIAL:
        case CDATETYPE:
        case SQLDATE:
        case CDTIMETYPE:
        case SQLDTIME:
        case CINVTYPE:
        case SQLINTERVAL:
            len = rtypwidth(col->sqltype & ~SQLDISTINCT, col->sqllen);
            break;

        case CLVCHARPTRTYPE:
        case CLOCATORTYPE:
        case SQLTEXT:
        case SQLBYTES:
            len = 0;
            break;

        case CBOOLTYPE:
        case SQLBOOL:
            len = 1;
            break;

        default:
            len = 0;
            cmd_error(E_INTERNAL, __FILE__ ":print_null() - unknown type for column");
            break;
        }

        blankput(len);
    }
}

/* Print non-null value */
static void print_value(const Sqlva *col)
{
    char buffer[BUFFERSIZE];
    char *str;
    int p;

    switch (col->sqltype & ~SQLDISTINCT)
    {
    case CDECIMALTYPE:
    case CMONEYTYPE:
    case SQLDECIMAL:
    case SQLMONEY:
        if (f_fixvar == F_VARIABLE)
        {
            /* General output format */
            /*
            ** NB: dectoasc() blank pads and does not null terminate its
            ** output, but ldchar() unpads and null terminates the string.
            **
            ** NBB: IDS 11.10.xC1 generates some profoundly odd type
            ** descriptions for decimal literals, such as DECIMAL(38,38)
            ** for 3.1415E-34 and DECIMAL(35,0) for 3.1415E+34.  Although
            ** there is some logic behind these, the descriptions should
            ** also be constrained by the maxima for the underlying
            ** DECIMAL data structures.
            */
            p = PRECDEC(col->sqllen);
            if (p == 0xFF)
                p = -1;
            dectoasc((dec_t *)col->sqldata, buffer, sizeof(buffer) - 1, p);
            buffer[sizeof(buffer) - 1] = '\0';
            ldchar(buffer, sizeof(buffer) - 1, buffer);
            str = buffer;
        }
        else
        {
            /* Fixed width output format, with trailing zeroes.  */
            if (PRECDEC(col->sqllen) == 0xFF)
            {
                /* Floating point */
                (void)dec_sci((dec_t *)col->sqldata, PRECTOT(col->sqllen), 0,
                              buffer, sizeof(buffer));
                str = rjust(buffer, PRECTOT(col->sqllen) + 7);
            }
            else
            {
                /* Fixed point */
                (void)dec_fix((dec_t *)col->sqldata, PRECDEC(col->sqllen), 0,
                              buffer, sizeof(buffer));
                str = rjust(buffer, PRECTOT(col->sqllen) + 1 +
                                        (PRECDEC(col->sqllen) > 0));
            }
        }
        output(str, QUOTE_NONE);
        break;

    case CFLOATTYPE:
    case SQLSMFLOAT:
        esnprintf(buffer, sizeof(buffer), fmt[FMT_FLOAT], *(float *)col->sqldata);
        output(buffer, QUOTE_NONE);
        break;

    case CDOUBLETYPE:
    case SQLFLOAT:
        esnprintf(buffer, sizeof(buffer), fmt[FMT_DOUBLE], *(double *)col->sqldata);
        output(buffer, QUOTE_NONE);
        break;

    case CSHORTTYPE:
    case SQLSMINT:
        esnprintf(buffer, sizeof(buffer), fmt[FMT_SHORT], *(ixInt2 *)col->sqldata);
        output(buffer, QUOTE_NONE);
        break;

    case CINTTYPE:
        /* Is this case ever used? */
        esnprintf(buffer, sizeof(buffer), fmt[FMT_INTEGER], *(ixMint *)col->sqldata);
        output(buffer, QUOTE_NONE);
        break;

    case CLONGTYPE:
    case SQLINT:
    case SQLSERIAL:
        esnprintf(buffer, sizeof(buffer), fmt[FMT_INTEGER], *(ixInt4 *)col->sqldata);
        output(buffer, QUOTE_NONE);
        break;

    case CSTRINGTYPE:
    case CFIXCHARTYPE:
    case SQLCHAR:
#ifdef SQLNCHAR
    case SQLNCHAR:
#endif /* SQLNCHAR */
        outstr(col->sqldata, col->sqllen);
        break;

    case CCHARTYPE:
        /* Strip trailing blanks */
        outstr(col->sqldata, col->sqllen - 1);
        break;

#ifdef SQLNVCHAR
    case SQLNVCHAR:
#endif /* SQLNVCHAR */
    case CVCHARTYPE:
    case SQLVCHAR:
        outvarchar(col);
        break;

    case CDATETYPE:
    case SQLDATE:
        rfmtdate(*(ixInt4 *)col->sqldata, ctxt_getdate(), buffer);
        if (f_fixvar == F_FIXED)
            jlss_stchar(buffer, buffer, strlen(ctxt_getdate()) + 1);
        output(buffer, QUOTE_BOTH);
        break;

    case CDTIMETYPE:
    case SQLDTIME:
    case CINVTYPE:
    case SQLINTERVAL:
        if ((col->sqltype & ~SQLDISTINCT) == CDTIMETYPE || (col->sqltype & ~SQLDISTINCT) == SQLDTIME)
            dttoasc((Datetime *)col->sqldata, buffer);
        else
            intoasc((Interval *)col->sqldata, buffer);
        str = buffer;
        if (f_fixvar == F_VARIABLE)
            str = skipblanks(buffer);
        else
        {
            p = rtypwidth((col->sqltype & ~SQLDISTINCT), col->sqllen) + 1;
            jlss_stchar(buffer, buffer, p);
        }
        output(str, QUOTE_BOTH);
        break;

    case CLOCATORTYPE:
    case SQLTEXT:
    case SQLBYTES:
        outblob((Blob *)col->sqldata);
        break;

#ifdef ESQLC_IUSTYPES
    case CLVCHARPTRTYPE:
    case SQLUDTVAR:
    case SQLUDTFIXED:
    case SQLLVARCHAR:
        outlvarchar(col);
        break;

    case CINT8TYPE:
    case SQLINT8:
    case SQLSERIAL8:
        outint8((ifx_int8_t *)col->sqldata);
        break;

    case CBOOLTYPE:
    case SQLBOOL:
        outboolean(col->sqldata);
        break;
#endif /* ESQLC_IUSTYPES */

#ifdef ESQLC_BIGINT
    case CBIGINTTYPE:
    case SQLINFXBIGINT:
    case SQLBIGSERIAL:

        esnprintf(buffer, sizeof(buffer), fmt[FMT_BIGINT], *(ixInt8 *)col->sqldata);
        output(buffer, QUOTE_NONE);

        // outbigint(*(bigint *)col->sqldata);
        break;
#endif /* ESQLC_BIGINT */

    default:
        cmd_error(E_INTERNAL, __FILE__ ":print_null() - unknown type for column");
        break;
    }
}

/* Print one column of data */
static void print_sqlva(const Sqlva *col)
{
    /*if (risnull(col->sqltype, col->sqldata))*/
    if (*col->sqlind != 0)
        print_null(col);
    else
        print_value(col);
}

static void sgml_opentag(const char *tag, const char *s)
{
    fprintf(ofile, "<%s>%s", tag, s);
}

static void sgml_closetag(const char *tag)
{
    fprintf(ofile, "</%s>%s", tag, eor);
}

/* Print one row of data in HTML (table) format */
static void print_sqlda_html(const Sqlda *desc)
{
    int i;
    Sqlva *col;

    sgml_opentag(html_tr, eor);
    for (i = desc->sqld, col = desc->sqlvar; i > 0; i--, col++)
    {
        sgml_opentag(html_td, "");
        print_sqlva(col);
        sgml_closetag(html_td);
    }
    sgml_closetag(html_tr);
    fputs(eor, ofile);
    fflush(ofile);
}

/* Print one row of data in markdown (table) format */
static void print_sqlda_markdown(const Sqlda *desc)
{
    int i;
    Sqlva *col;

    for (i = desc->sqld, col = desc->sqlvar; i > 0; i--, col++)
    {
        if (i == desc->sqld)
        {
            fprintf(ofile, "| ");
        }
        else
        {
            fprintf(ofile, " | ");
        }
        print_sqlva(col);
    }
    fprintf(ofile, " |%s", eor);

    fflush(ofile);
}

/* Print one row of data in XML format */
static void print_sqlda_xml(const Sqlda *desc)
{
    int i;
    Sqlva *col;

    sgml_opentag(ctxt_getxmlrecordtag(), eor);
    for (i = desc->sqld, col = desc->sqlvar; i > 0; i--, col++)
    {
        sgml_opentag(col->sqlname, "");
        print_sqlva(col);
        sgml_closetag(col->sqlname);
    }
    sgml_closetag(ctxt_getxmlrecordtag());
    fputs(eor, ofile);
    fflush(ofile);
}

/* Print one row of data in block format */
static void print_sqlda_block(const Sqlda *desc)
{
    int i;
    Sqlva *col;

    for (i = desc->sqld, col = desc->sqlvar; i > 0; i--, col++)
    {
        fprintf(ofile, "%-18s ", col->sqlname);
        print_sqlva(col);
        fputs(eor, ofile);
    }
    fputs(eor, ofile);
    fflush(ofile);
}

/* Print one row of data (regular, non-XML format) */
static void print_sqlda_reg(const Sqlda *desc)
{
    int i;
    Sqlva *col;

    for (i = desc->sqld, col = desc->sqlvar; i > 0; i--, col++)
    {
        print_sqlva(col);
        /* Print delimiter if required */
        if ((i > 1 || formt == OP_UNLOAD || formt == OP_FIXDEL) && delim != '\0' && formt != OP_FIXED)
            putc(delim, ofile);
    }
    fputs(eor, ofile);
    fflush(ofile);
}

/* Print one row of data */
int print_record(const Sqlda *desc)
{
    if (formt == OP_XML)
        print_sqlda_xml(desc);
    else if (formt == OP_HTML)
        print_sqlda_html(desc);
    else if (formt == OP_BLOCK)
        print_sqlda_block(desc);
    else if (formt == OP_MARKDOWN)
        print_sqlda_markdown(desc);
    else
        print_sqlda_reg(desc);
    return (io_error(ofile));
}

/* Set the given Sqlva to point to the given data as a character string */
static void set_sqlva_char(Sqlva *var, const char *data, size_t data_len)
{
    var->sqltype = SQLCHAR;
    var->sqldata = STRDUP(data);
    var->sqlind = &ind_notnull;
    var->sqllen = data_len;
    var->sqlname = "column_name";
}

/* Release space allocated for string */
static void rel_sqlva_char(Sqlva *var)
{
    FREE(var->sqldata);
    var->sqldata = 0;
}

/* Release space allocated for strings in Sqlda */
static void rel_sqlda_char_data(Sqlda *desc)
{
    int i;
    for (i = 0; i < desc->sqld; i++)
        rel_sqlva_char(&desc->sqlvar[i]);
}

/* Print a line containing the display labels of the columns */
static int print_heading(const Sqlda *desc)
{
    int i;
    Sqlda *names = alloc_sqlda(desc->sqld);

    reset_column_exp_count();
    for (i = 0; i < desc->sqld; i++)
    {
        Sqlva *sqlv = &desc->sqlvar[i];
        const char *col_name = column_name(sqlv);
        set_sqlva_char(&names->sqlvar[i], col_name, strlen(col_name));
    }
    print_record(names);
    rel_sqlda_char_data(names);
    dealloc_sqlda(names);

    return (io_error(ofile));
}

/* Print a line containing the types of the columns */
static int print_types(const Sqlda *desc)
{
    int i;
    Sqlda *types = alloc_sqlda(desc->sqld);

    for (i = 0; i < desc->sqld; i++)
    {
        Sqlva *sqlv = &desc->sqlvar[i];
        const char *type_name = sqltype((sqlv->sqltype & ~SQLDISTINCT), sqlv->sqllen);
        set_sqlva_char(&types->sqlvar[i], type_name, strlen(type_name));
    }
    print_record(types);
    rel_sqlda_char_data(types);
    dealloc_sqlda(types);

    return (io_error(ofile));
}

/*
** print_htmlheading() - Print an HTML heading records
** Contains display labels always, and type information on request
*/
static int print_htmlheading(const Sqlda *desc)
{
    int i;

    reset_column_exp_count();

    sgml_opentag(html_tr, eor);
    for (i = 0; i < desc->sqld; i++)
    {
        sgml_opentag(html_th, "");
        output(column_name(&desc->sqlvar[i]), QUOTE_NONE);
        sgml_closetag(html_th);
    }
    sgml_closetag(html_tr);

    if (ctxt_gettypes() == OP_ON)
    {
        sgml_opentag(html_tr, eor);
        for (i = 0; i < desc->sqld; i++)
        {
            Sqlva *sqlv = &desc->sqlvar[i];
            const char *type = sqltype((sqlv->sqltype & ~SQLDISTINCT), sqlv->sqllen);
            sgml_opentag(html_td, "");
            output(type, QUOTE_NONE);
            sgml_closetag(html_td);
        }
        sgml_closetag(html_tr);
    }

    fputs(eor, ofile);
    return (io_error(ofile));
}

/*
** print_markdowneading() - Print a github flavourd markdown heading
** this has and additional row after the header line where every field is '-' characters
*/
static int print_markdownheading(const Sqlda *desc)
{
    int i;

    reset_column_exp_count();

    for (i = 0; i < desc->sqld; i++)
    {
        if (i == 0)
        {
            fprintf(ofile, "| ");
        }
        else
        {
            fprintf(ofile, " | ");
        }
        output(column_name(&desc->sqlvar[i]), QUOTE_NONE);
    }
    fprintf(ofile, " |%s", eor);

    for (i = 0; i < desc->sqld; i++)
    {
        fprintf(ofile, "| - ");
    }
    fprintf(ofile, "|%s", eor);

    if (ctxt_gettypes() == OP_ON)
    {
        output(column_name(&desc->sqlvar[i]), QUOTE_NONE);
        for (i = 0; i < desc->sqld; i++)
        {
            Sqlva *sqlv = &desc->sqlvar[i];
            const char *type = sqltype((sqlv->sqltype & ~SQLDISTINCT), sqlv->sqllen);
            if (i == 0)
            {
                fprintf(ofile, "| ");
            }
            else
            {
                fprintf(ofile, " | ");
            }
            output(type, QUOTE_NONE);
        }
        fprintf(ofile, " |%s", eor);
    }

    return (io_error(ofile));
}

/*
** print_xmlheading() - Print an XML-ish heading record
** Contains display labels and type information for the columns
*/
static int print_xmlheading(const Sqlda *desc)
{
    int i;
    Sqlva *sqlv;
    const char *name;
    const char *type;

    reset_column_exp_count();
    sgml_opentag(ctxt_getxmlheadertag(), eor);
    for (i = 0; i < desc->sqld; i++)
    {
        sqlv = &desc->sqlvar[i];
        name = column_name(sqlv);
        type = sqltype(sqlv->sqltype, sqlv->sqllen);
        /* int cast stops gcc wittering about int format and int4 argument */
        fprintf(ofile, f_xmlheading_fmt, (i + 1), name, sqlv->sqltype, (int)sqlv->sqllen, type, eor);
    }
    sgml_closetag(ctxt_getxmlheadertag());
    fputs(eor, ofile);
    return (io_error(ofile));
}

/* Print headings appropriately */
int print_header(const Sqlda *desc)
{
    int rc = 0;
    int headings = ctxt_getheading();
    int types = ctxt_gettypes();

    set_printmode();
    if (formt == OP_XML)
    {
        fputs(f_xmlidstring, ofile);
        fputs(eor, ofile);
        sgml_opentag(ctxt_getxmlrecsettag(), eor);
        if (headings == OP_ON || types == OP_ON)
        {
            if (print_xmlheading(desc) != 0)
                rc = -1;
        }
    }
    else if (formt == OP_HTML)
    {
        sgml_opentag(html_tb, eor);
        print_htmlheading(desc);
    }
    else if (formt == OP_MARKDOWN)
    {
        print_markdownheading(desc);
    }
    else
    {
        if (headings == OP_ON)
        {
            if (print_heading(desc) != 0)
                rc = -1;
        }
        if (rc == 0 && types == OP_ON)
        {
            if (print_types(desc) != 0)
                rc = -1;
        }
    }
    return (rc);
}

/* Print trailing information */
void print_trailer(void)
{
    if (formt == OP_XML)
        sgml_closetag(ctxt_getxmlrecsettag());
    else if (formt == OP_HTML)
        sgml_closetag(html_tb);
}
