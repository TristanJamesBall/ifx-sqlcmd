/*
@(#)File:           $RCSfile: readload.c,v $
@(#)Version:        $Revision: 2015.1 $
@(#)Last changed:   $Date: 2015/11/09 01:37:12 $
@(#)Purpose:        Read and convert record from LOAD file
@(#)Author:         J Leffler
@(#)Copyright:      (C) JLSS 1998-2005,2008-12
@(#)Product:        SQLCMD Version 90.02 (2016-07-28)
*/

/*TABSTOP=4*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "kludge.h"
#include "context.h"
#include "esqlc.h"
#include "memory.h"
#include "sqlcmd.h"
#include "sqlerr.h"
#include "jlsstools.h"
#include "stderr.h"
#include "esqlutil.h"
#include "debug.h"
#include "dumpesql.h"

/*
** For production work, don't want to have to do multiple jlss_getline() calls.
** For development work, want to test the multiple jlss_getline() calls easily.
*/
#ifdef DEBUG
#define READRECBUFSIZE  32
#else
#define READRECBUFSIZE  8192
#endif

/* Codes for handling backslash-space as empty, non-null VARCHAR */
typedef enum BSSP { BSSP_PRESENT = 0x4321, BSSP_MISSING, BSSP_UNKNOWN } BSSP;

/* Function pointer for scanning a field within a record */
typedef int  (*FieldScanner)(Sqlva *p_col, Memory *p_line, Memory *p_field, int p_fnum,
                             Uchar p_escape, Uchar p_delim, Uchar p_quote, BSSP *bssp);

#define BYTEBLOB(x)     ((x) == CLOCATORTYPE || (x) == SQLBYTES)
#define TEXTBLOB(x)     ((x) == SQLTEXT)
#undef MIN      /* 2005-04-06: HP-UX 11i - precaution (see history.c) */
#define MIN(a, b)       (((a) < (b)) ? (a) : (b))

#ifndef lint
/* Prevent over-aggressive optimizers from eliminating ID string */
const char jlss_id_readload_c[] = "@(#)$Id: readload.c,v 2015.1 2015/11/09 01:37:12 jleffler Exp $";
#endif /* lint */

static double ustrtod(const Uchar *s, Uchar **e)
{
    return strtod((const char *)s, (char **)e);
}

static long ustrtol(const Uchar *s, Uchar **e, int b)
{
    return strtol((const char *)s, (char **)e, b);
}

void rec_zero(RecLoc *rec)
{
    /* Sometime - measure this against memset(rec, 0, sizeof(RecLoc)); */
    static RecLoc const zero = { 0 };
    *rec = zero;
}

/*
**  Convert the data in a memory block into the correct type.
*/
static int convertdata(Memory *mem, Sqlva *col, BSSP bssp)
{
    enum { null = -1, notnull = 0 };
    size_t          len;
    ixInt4          lval = 0;
    double          dval = 0.0;
    int             retval;
    ixInt2          type = 0;
    const char     *function = "<undefined>";
    Uchar          *end;
    ixInt4         *date;
    Decimal        *dcp;
    Datetime       *dtp;
    Interval       *inp;
    Blob           *blob;
    FUNCTION_NAME("convertdata");

    TRACE((5, "-->> convertdata()\n"));

    retval = sqlca.sqlcode = 0;
    len = mem_len(mem);
    if (len == 0)
    {
        TRACE((5, "null because zero length\n"));
        *col->sqlind = null;
    }
    else if (len == 1 && *mem_data(mem) == '\0' && !BYTEBLOB(col->sqltype))
    {
        TRACE((5, "null because length one and data ASCII NUL\n"));
        *col->sqlind = null;
    }
    else
    {
        TRACE((5, "data not null '%s'\n", mem_data(mem)));
        *col->sqlind = notnull;
        switch (type = col->sqltype)
        {
        case CCHARTYPE:
        case CSTRINGTYPE:
        case CFIXCHARTYPE:
        case SQLCHAR:
#ifdef SQLNCHAR
        case SQLNCHAR:
#endif /* SQLNCHAR */
            /* Should we warn if string is truncated?? */
            len = MIN(mem_len(mem), (size_t)col->sqllen);
            strncpy(col->sqldata, (const char *)mem_data(mem), len);    /*=C++=*/
            break;

        case CVCHARTYPE:
        case SQLVCHAR:
#ifdef SQLNVCHAR
        case SQLNVCHAR:
#endif /* SQLNVCHAR */
            /* Should we warn if string is truncated?? */
            if (bssp == BSSP_PRESENT)
                len = 0;
            else
                len = MIN(mem_len(mem), (size_t)col->sqllen);
            strncpy(col->sqldata, (const char *)mem_data(mem), len);    /*=C++=*/
            col->sqldata[len] = '\0';
            break;

#ifdef ESQLC_IUSTYPES
        case CLVCHARPTRTYPE:
        case SQLLVARCHAR:
            {
            /* NB: ifx_var_setdata() sets sqlca.sqlcode on error! */
            /* Workaround for B169213 - ifx_var_setdata() won't accept zero length data */
            void *data = col->sqldata;
            char *memd = (char *)mem_data(mem); /*=C++=*/
            TRACE((5, "---- %s: mem_len %d, sqllen %d\n", __func__, (int)mem_len(mem), (int)col->sqllen));
            /*
            ** JL 2008-07-13: sql_describe() sets col->sqllen to
            ** sizeof(void *) instead of size of column, so the MIN() line
            ** does not work well.  Hence, remove it, pro tem.
            */
            /*len = MIN(mem_len(mem), (size_t)col->sqllen);*/
            len = mem_len(mem);
            /* Don't include the trailing ASCII NUL in the LVARCHAR value. */
            if (len > 0 && memd[len-1] == '\0')
                len--;
            if (ifx_var_setdata(&data, memd, len) == 0 && bssp == BSSP_PRESENT)
                ifx_var_setlen(&data, 0);
            /* Ensure safety if col->sqldata was initially unallocated */
            col->sqldata = (char *)data;    /*=C++=*/
            }
            break;

        case CBOOLTYPE:
        case SQLBOOL:
            {
                char data = *mem_data(mem);
                if (mem_len(mem) > 2)
                    error_746("string is too long to convert to BOOLEAN");
                else
                {
                    switch (data)
                    {
                    case 't':
                    case 'T':
                        *col->sqldata = 1;
                        break;
                    case 'f':
                    case 'F':
                        *col->sqldata = 0;
                        break;
                    default:
                        error_746("string value cannot be converted to BOOLEAN");
                        break;
                    }
                }
            }
            break;
#endif /* ESQLC_IUSTYPES */

        case CSHORTTYPE:
        case SQLSMINT:
            lval = ustrtol(mem_data(mem), &end, ctxt_getibase());
            function = "ustrtol";
            type = SQLSMINT;
            if (end == mem_data(mem) || *end != '\0')
            {
                /* Character to numeric conversion error */
                sqlca.sqlcode = -1213;
            }
            else if (lval < -32767 || lval > 32767)
            {
                /* Value too large to fit in a SMALLINT */
                sqlca.sqlcode = -1214;
            }
            else
                *((ixInt2 *)col->sqldata) = (ixInt2)lval;
            break;

        case CINTTYPE:
        case CLONGTYPE:
        case SQLINT:
        case SQLSERIAL:
            lval = ustrtol(mem_data(mem), &end, ctxt_getibase());
            if (end == mem_data(mem) || *end != '\0')
            {
                /* Character to numeric conversion error */
                sqlca.sqlcode = -1213;
                function = "ustrtol";
                type = SQLINT;
            }
            else
                *((ixInt4 *)col->sqldata) = lval;
            break;

        case CFLOATTYPE:
        case SQLSMFLOAT:
            dval = ustrtod(mem_data(mem), &end);
            if (end == mem_data(mem) || *end != '\0')
            {
                /* Character to numeric conversion error */
                sqlca.sqlcode = -1213;
                function = "strtod";
                type = SQLSMFLOAT;
            }
            else
                *(float *)col->sqldata = (float)dval;
            break;

        case CDOUBLETYPE:
        case SQLFLOAT:
            dval = ustrtod(mem_data(mem), &end);
            if (end == mem_data(mem) || *end != '\0')
            {
                /* Character to numeric conversion error */
                sqlca.sqlcode = -1213;
                function = "strtod";
                type = SQLFLOAT;
            }
            else
                *(double *)col->sqldata = dval;
            break;

        case CMONEYTYPE:
        case SQLMONEY:
        case CDECIMALTYPE:
        case SQLDECIMAL:
            /* Is the scale & precision set correctly? */
            dcp = (Decimal *)col->sqldata;
            sqlca.sqlcode = deccvasc(mem_cdata(mem), len, dcp);
            if (sqlca.sqlcode != 0)
            {
                function = "deccvasc";
                type = (type == CMONEYTYPE || type == SQLMONEY)
                        ? SQLMONEY : SQLDECIMAL;
            }
            break;

        case CDATETYPE:
        case SQLDATE:
            date = (ixInt4 *)col->sqldata;
            sqlca.sqlcode = rdefmtdate(date, ctxt_getdate(), mem_cdata(mem));
            if (sqlca.sqlcode != 0)
            {
                function = "rdefmtdate";
                type = SQLDATE;
            }
            break;

        case CDTIMETYPE:
        case SQLDTIME:
            dtp = (Datetime *)col->sqldata;
            dtp->dt_qual = (ixInt2)col->sqllen;
            sqlca.sqlcode = dtcvasc(mem_cdata(mem), dtp);
            if (sqlca.sqlcode != 0)
            {
                function = "dtcvasc";
                type = SQLDTIME;
            }
            break;

        case CINVTYPE:
        case SQLINTERVAL:
            inp = (Interval *)col->sqldata;
            inp->in_qual = (ixInt2)col->sqllen;
            sqlca.sqlcode = incvasc(mem_cdata(mem), inp);
            if (sqlca.sqlcode != 0)
            {
                function = "incvasc";
                type = SQLINTERVAL;
            }
            break;

        case CLOCATORTYPE:
        case SQLTEXT:
        case SQLBYTES:
            blob = (Blob *)col->sqldata;
            blob->loc_loctype = LOCMEMORY;
            blob->loc_bufsize = mem_len(mem);
            blob->loc_buffer = mem_cdata(mem);
            blob->loc_size = len;
            blob->loc_indicator = (len == 0) ? -1 : 0;
            /* Invalidate memory block 'cos blob is using it! */
            mem_zap(mem);
            mem_rst(mem);
            break;

#ifdef ESQLC_IUSTYPES
        case CINT8TYPE:
        case SQLINT8:
        case SQLSERIAL8:
            sqlca.sqlcode = ifx_int8cvasc(mem_cdata(mem), mem_len(mem)-1, (ifx_int8_t *)col->sqldata);
            break;
#endif /* ESQLC_IUSTYPES */

#ifdef ESQLC_BIGINT
        case CBIGINTTYPE:
        case SQLINFXBIGINT:
        case SQLBIGSERIAL:
            sqlca.sqlcode = bigintcvasc(mem_cdata(mem), mem_len(mem)-1, (bigint *)col->sqldata);
            break;
#endif /* ESQLC_BIGINT */

        default:
            err_remark("unknown type -- (%" PRId_ixInt2 ", %" PRId_ixInt4 ") %s\n",
                       col->sqltype, col->sqllen,
                       sqltype(col->sqltype, col->sqllen));
            cmd_error(E_INTERNAL, __FILE__ ":convertdata() - unknown type");
            break;
        }
    }

    if (sqlca.sqlcode < 0)
    {
        err_remark("%s(): error %" PRId_ixInt4 " for column %s converting '%s' to %s\n",
                   function, sqlca.sqlcode, col->sqlname, mem_data(mem),
                   sqltype(type, col->sqllen));
        retval = -1;
    }
    TRACE((5, "<<-- convertdata() rv = %d\n", retval));
    return(retval);
}

/* hexdigit() - convert character to corresponding hexadecimal value */
static int hexdigit(Uchar c)
{
    int n = 0;

    assert(isxdigit(c));
    if (isdigit(c))
        n = c - '0';
    else if (c >= 'a' && c <= 'f')
        n = c - 'a' + 10;
    else if (c >= 'A' && c <= 'F')
        n = c - 'A' + 10;
    else
        assert(0);
    return(n);
}

static void load_errline(RecLoc *rec, const char *name)
{
    if (rec->rownum == rec->line_2)
    {
        err_remark("error in line %d of file %s\n", (int)rec->rownum, name);
    }
    else
    {
        err_remark("error in row %d (lines %d..%d) of file %s\n",
                   (int)rec->rownum, (int)rec->line_1, (int)rec->line_2, name);
    }
}

/*
** readrecord - read single record into memory.
** See unload.format document for description of record layout.
** On entry, the RecLoc structure contains the description of the
** last record read -- the row number and the range of lines it occupied.
** If this is the first record, then all the RecLoc elements are zero.
** If we read any data, then the row number is increased and the range of
** lines is initially set to one more than the last line of the last record.
** As more lines of data are added, the last line number is incremented.
** Note, however, that jlss_getline() can return successfully either because it
** read a whole line, or because it ran out of space to store the data.
*/
static int readrecord(FILE *fp, const char *name, RecLoc *rec, Memory *m, char escape)
{
    Uchar buffer[READRECBUFSIZE];
    size_t line_1 = rec->line_2 + 1;
    int nbytes;

    mem_rst(m);
    while ((nbytes = jlss_getline(fp, (char *)buffer, sizeof(buffer))) != EOF)
    {
        mem_addstr(m, buffer, nbytes);
        if (mem_last(m) == '\n')
        {
            const Uchar *m_end;
            const Uchar *m_src;
            size_t len = mem_len(m);
            size_t e_cnt = 0;   /* escape count - number of trailing backslashes */
            rec->line_2++;
            assert(len != 0);
            if (len == 1)
            {
                assert(buffer[0] == '\n');
                break;      /* Empty line (newline only); EOR */
            }
            m_src = mem_data(m);
            m_end = m_src + len - 2;
            assert(m_end[1] == '\n');
            if (m_end[0] == escape)
            {
                /*
                ** NB: non-escape, escape, newline marks continuation,
                ** but non-escape, escape, escape, newline marks EOL,
                ** and non-escape, escape, escape, escape, newline marks continuation,
                ** and so on...worry about it!
                */
                e_cnt = 1;
                while (m_end > m_src && *--m_end == escape)
                    e_cnt++;
            }
            if (e_cnt % 2 == 0)
                break;
        }
    }
    if (feof(fp) && mem_len(m) == 0)
        return(EOF);    /* No data read */
    rec->rownum++;
    rec->line_1 = line_1;
    if (feof(fp) && mem_last(m) != '\n')
    {
        /* EOF with data but without trailing newline, perhaps */
        /* because second last character is escape (backslash) */
        mem_add(m, '\n');
        rec->line_2++;
        load_errline(rec, name);
        cmd_warning(E_EOFWONL, name);
    }
    mem_end(m, '\0');
    assert(rec->line_1 <= rec->line_2);
    assert(rec->rownum <= rec->line_1);
    assert(rec->rownum != 0);
    assert(rec->line_1 != 0);
    assert(rec->line_2 != 0);
    return(0);  /* Data was read so not EOF yet */
}

static Uchar from_hex(int c0, int c)
{
    return (Uchar)((hexdigit((Uchar)c0) << 4) | hexdigit((Uchar)c));
}

/*
** Scan a field for hex encoded BYTE blob data
** KLUDGE: some cleanup still needed - escapes not relevant (permitted),
**         or only to escape a newline?
*/
static int scanbytefield(Sqlva *col, Memory *line, Memory *field, int fnum,
                         Uchar escape, Uchar delim, Uchar quote, BSSP *bssp)
{
    int parity = 0;
    int c;

    assert(BYTEBLOB(col->sqltype));

    /* BYTE blobs do not support the backslash-space convention */
    *bssp = BSSP_MISSING;

    while ((c = mem_char(line)) != EOF)
    {
        if (c == delim)
        {
            /* Field delimiter */
            break;
        }
        else if (c == escape)
        {
            /* Escape character -- use next character */
            if ((c = mem_char(line)) == EOF)
                break;
        }
        else if (c == quote)
        {
            /* Un-escaped quote -- ignore */
            assert(ctxt_getformat() == OP_QUOTE);
            continue;
        }
        else if (c == '\n')
        {
            /* Final field delimiter is optional */
            break;
        }
        if ((++parity & 0x1) == 0)
        {
            int c0 = mem_pop(field);
            if (!isxdigit(c0) || !isxdigit(c))
            {
                sqlca.sqlcode =  -32405;    /* Incorrectly formed hexadecimal value */
                break;
            }
            mem_add(field, from_hex(c0, c));
        }
        else
            mem_add(field, (Uchar)c);
    }

    if ((parity & 0x01) != 0)
        sqlca.sqlcode =  -32405;    /* Incorrectly formed hexadecimal value */
    TRACE((5, "Field %d: BLOB DATA - not printed\n", fnum));

    return c;
}

/*
** Scan a CSV quote-enclosed field after opening quote read.
** An embedded quote quote sequence means a single quote.
*/
static int scanquote(Memory *line, Memory *field, Uchar delim, Uchar quote)
{
    int c;
    int eor = '\n';

    while ((c = mem_char(line)) != EOF)
    {
      if (c == quote)
        {
         /* Check for doubled quote, etc. */
         if ((c = mem_peek(line)) == quote)
         {
            c = mem_char(line);
            mem_add(field, (Uchar)c);
         }
         else
         {
            /* Found a 'closing quote'; ignore it and scan to field delimiter */
            /* NB: also completes record at EOR (newline).  Kludgy... */
            while ((c = mem_char(line)) != EOF && c != delim && c != eor)
            {
               mem_add(field, (Uchar)c);
            }
            break;
        }
      }
      else
         mem_add(field, (Uchar)c);
   }
   return c;
}

/*
** Scan a regular field delimited by delim.
** Special characters (newline, escape, delim) escaped by escape.
** If ctxt_gethexmode() returns true, treat \AB (where A and B are hex
** digits) as a single char, for compatibility with DB-Access (more or
** less -- DB-Access treats LOAD and UNLOAD specially, but not regular
** output).
*/
static int scanfield(Memory *line, Memory *field, Uchar escape, Uchar delim, BSSP *bssp)
{
    int c;

    while ((c = mem_char(line)) != EOF && c != delim)
    {
        if (c == escape)
        {
            /* Escape character -- use next character */
            if ((c = mem_char(line)) == EOF)
                break;
            if (*bssp == BSSP_UNKNOWN && c == ' ')
                *bssp = BSSP_PRESENT;
            else if (isxdigit(c) && ctxt_gethexmode())
            {
                int c2;
                if ((c2 = mem_peek(line)) != EOF && isxdigit(c2))
                {
                    c2 = mem_char(line);
                    c = hexdigit(c) * 16 + hexdigit(c2);
                }
            }
        }
        else if (c == '\n')      /* Bug: should be EOR? */
        {
            /* Final field delimiter is optional */
            break;
        }
        mem_add(field, (Uchar)c);
    }
    return(c);
}

/*
** Scan a CSV field for non-BYTE data.
** In CSV (OP_QUOTE) mode, there are two formats to process - regular
** fields and quoted fields.  Assuming standard setups, "ABC,DEF" is a
** 7-character field; """ABC,DEF""" is a 9-character field with quotes
** around the value.  It isn't wholly clear what should happen if you
** encounter a field value such as "ABC,DE"FG, which is clearly
** malformed (see also the discussion in Kernighan and Pike "The
** Practice of Programming").  The best option is probably to treat the
** field value as ABC,DEFG - though an error would not be unreasonable
** either.  Also, A"B,C"D,E parses as three fields, A"B and C"D and E.
** That is, the quote is only special when it is the first character.
** In a CSV file, the escape character (backslash) has no significance.
** At least on MacOS X, Excel permits a quoted field to continue onto
** the next line - up to the matching quote.  The logic here does not
** do that unless the newline is preceded by a backslash.
*/
static int scancsvfield(Sqlva *col, Memory *line, Memory *field, int fnum,
                        Uchar escape, Uchar delim, Uchar quote, BSSP *bssp)
{
   int c;

   assert(!BYTEBLOB(col->sqltype));

   *bssp = BSSP_UNKNOWN;
   if ((c = mem_peek(line)) != EOF)
   {
      if (c == quote)
      {
         *bssp = BSSP_MISSING;
         c = mem_char(line);
         c = scanquote(line, field, delim, quote);
      }
      else
         c = scanfield(line, field, escape, delim, bssp);
   }

   if (*bssp == BSSP_UNKNOWN || mem_len(field) > 1)
      *bssp = BSSP_MISSING;

   if (TEXTBLOB(col->sqltype))
      mem_end(field, '\0');
   else
      mem_add(field, '\0');
   TRACE((5, "Field %d: <<%s>>\n", fnum, mem_cdata(field)));

   return c;
}

/*
** Scan a field for text (non-BYTE) data.
*/
static int scantextfield(Sqlva *col, Memory *line, Memory *field, int fnum,
                         Uchar escape, Uchar delim, Uchar quote, BSSP *bssp)
{
    int c;

    assert(!BYTEBLOB(col->sqltype));

    *bssp = BSSP_UNKNOWN;
    c = scanfield(line, field, escape, delim, bssp);

    if (*bssp == BSSP_UNKNOWN || mem_len(field) > 1)
        *bssp = BSSP_MISSING;

    if (TEXTBLOB(col->sqltype))
        mem_end(field, '\0');
    else
        mem_add(field, '\0');
    TRACE((5, "Field %d: <<%s>>\n", fnum, mem_cdata(field)));

    return c;
}

/*
** scanrecord() - read data line from file according to sqlda structure
**
** fp       - file to read
** idesc    - controlling format
** rec      - in/out: record location (rownum, line_1, line_2)
** name     - input file name
** line     - where line should be returned
**
** Returns:
** 0   - data read
**     - sqlca.sqlcode == 0 => all OK
**     - sqlca.sqlcode != 0 => some data format error
** EOF - no data to read
**
** NB: It would be nice to avoid copying the data from line into field.
** However, any field can contain escape characters, especially if the escape
** is weird (such as a digit).  So, in general, we have to copy the data.
** Adding code to determine whether we might be able to avoid copying the data
** (such as by looking at the values in rec to see whether the record spreads
** over several lines) might help.  If might be faster to use strchr() to
** identify whether any escape characters occur, and use non-copying processing
** if they don't, but you'd have to do some benchmarking to prove your point.
** Note that version 2.6 went a long way down the non-copying road, but ran
** foul of the escape characters, and hence version 2.7 reverted to the same
** processing as in version 2.5 and earlier.
*/
int scanrecord(FILE *fp, Sqlda *idesc, RecLoc *rec, const char *name, Memory *line)
{
    int     i;
    int     c;
    Uchar   delim;
    Uchar   escape = ctxt_getescape();
    Uchar   quote;
    Memory  field;
    BSSP    bssp;

    TRACE((5, "-->> scanrecord() - sqlda = 0x%08lX\n", (unsigned long)idesc));
    if (db_getdebug() >= 8)
        dump_sqlda(stdout, "---- scanrecord()", idesc);

    /* Attempt to read a record into line */
    if (readrecord(fp, name, rec, line, escape) == EOF)
    {
        TRACE((5, "<<-- scanrecord() - EOF\n"));
        return(EOF);
    }

    /* Some data was read... and has a newline at the end */
    TRACE((5, "New Record - Row=%d(Lines=%d..%d;len=%d):%s", rec->rownum,
            rec->line_1, rec->line_2, mem_len(line), mem_data(line)));

    delim = ctxt_getdelim();
    if (ctxt_getformat() == OP_QUOTE)
        quote = ctxt_getquote();
    else
        quote = escape;

    mem_new(&field);
    sqlca.sqlcode = 0;

    mem_scan(line); /* Set structure for scanning */
    for (i = 0; i < idesc->sqld; i++)
    {
        Sqlva *col = &idesc->sqlvar[i];
        FieldScanner scanner;

        mem_rst(&field);
        if (BYTEBLOB(col->sqltype))
            scanner = scanbytefield;
        else if (ctxt_getformat() == OP_QUOTE)
            scanner = scancsvfield;
        else
            scanner = scantextfield;
        c = (*scanner)(col, line, &field, i, escape, delim, quote, &bssp);
        if (sqlca.sqlcode != 0)
            break;
        else if (c == delim || ((c == '\n' || c == EOF) && i == idesc->sqld - 1))
        {
            if (convertdata(&field, col, bssp) != 0)
                load_errline(rec, name);
            TRACE((5, "col = 0x%08lX, sqlind = %d\n", (unsigned long)col, *col->sqlind));
        }
        else
        {
            /* Number of values in file != number of columns */
            load_errline(rec, name);
            sqlca.sqlcode = -846;
            set_isam_err(-746, "Too few values in record");
        }
    }

    if ((c = mem_char(line)) != EOF && c != '\n')
    {
        load_errline(rec, name);
        sqlca.sqlcode = -846;   /* No. values in file != columns */
        set_isam_err(-746, "Too many values in record");
    }
    mem_del(&field);
    TRACE((5, "<<-- scanrecord()\n"));
    return(0);
}
