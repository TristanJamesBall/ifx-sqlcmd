/*
@(#)File:           $RCSfile: iustoken.c,v $
@(#)Version:        $Revision: 2016.1 $
@(#)Last changed:   $Date: 2016/01/17 19:21:46 $
@(#)Purpose:        Informix SQL tokenizer that handles SET{ROW('a','b')}
@(#)Author:         J Leffler
@(#)Copyright:      (C) JLSS 2000-01,2003-04,2006,2008,2016
@(#)Product:        SQLCMD Version 90.02 (2016-07-28)
*/

/*TABSTOP=4*/

#include "sqltoken.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <ctype.h>
#include <string.h>
#include "tokencmp.h"   /* tokencmp */

#undef DEBUG
#include "debug.h"  /* TRACE() etc */

#define CONST_CAST(type, value) ((type)(value))

#define LCURLY  '{'

#ifndef lint
/* Prevent over-aggressive optimizers from eliminating ID string */
extern const char jlss_id_iustoken_c[];
const char jlss_id_iustoken_c[] = "@(#)$Id: iustoken.c,v 2016.1 2016/01/17 19:21:46 jleffler Exp $";
#endif /* lint */

/*
** Determine whether given token is a collection name
*/
static int collection_name(const char *token, size_t len)
{
    int rc = 0;
    DB_TRACKING();
    if (isalpha((unsigned char)*token) == 0)
        rc = 0;
    else if (len == sizeof("SET")-1 && tokencmp(token, len, "SET", len) == 0)
        rc = 1;
    else if (len == sizeof("LIST")-1 && tokencmp(token, len, "LIST", len) == 0)
        rc = 1;
    else if (len == sizeof("MULTISET")-1 && tokencmp(token, len, "MULTISET", len) == 0)
        rc = 1;
    return rc;
}

/*
** iustoken() - get SQL token (or start of IUS collection literal) from string
**
** Returns a pointer to the start of the next SQL token (keyword, string,
** punctuation), skipping comments and blanks, or a pointer to the ASCII
** NUL at end of string if there are no more tokens.  The first character
** after the end of the token is returned in the end parameter.
**
** The code here depends on sqltoken() to recognize comments, strings,
** and so on - check that file for details on the basic recognizer.
**
** IUS Collection Types are a magnificent pain in the posterior:
**    INSERT INTO Foo
**       SELECT Id, Val_One, Val_Two
**          FROM TABLE(SET{ROW(1, 'Paul','Paul'),
**                         ROW(2, 'Paul ', 'Paul '),
**                         ROW(3, 'Paul  ','Paul  ')
**                        }::SET(ROW(Id INTEGER,
**                                   Val_One CHAR(10),
**                                   Val_Two VARCHAR(10) ) NOT NULL)
**                );
** In this context, the braces do not mark a comment; they are part of the
** SET syntax.  What's worse, it is not clear how a general purpose routine
** like sqltoken() can identify when the braces are going to be a comment
** and when they are syntactically critical.  I wonder if you can have a
** brace-comment inside a SET definition?  In general, you can have all
** sorts of junk inside a comment; unbalanced quotes, stray open braces and
** so on do not matter.  But inside a SET{...}, everything has to be
** kosher.  If the parser looks for Perl regexp /[Ss][Ee][Tt]\w*{/ as a
** single token, then it can be made to work (but presumably, that notation
** applies to MULTISET and LIST too?).  Strictly, similar rules apply to
** INTERVAL( and DATETIME( in Informix -- though the standard uses strings
** rather than parentheses to enclose the literal!
**
** Whoever it was that decided to use braces for syntactically critical
** constructs of Informix's dialect of SQL should be hung, drawn and
** quartered!
*/

char *iustoken(const char *input, const char **end)
{
    const char *token;
    size_t len;

    DB_TRACE(0, "-->>iustoken: <<%.32s%s>>\n", input, (strlen(input) > 32 ? "..." : ""));
    token = sqltoken(input, end);
    len = *end - token;
    if (collection_name(token, len))
    {
        /**
        ** Token is SET, MULTISET or LIST.
        ** Now check whether next non-blank character not in a comment
        ** is open brace.  If so, the construct is the start of an IDS
        ** collection literal.
        ** NB: Does handle double-dash comments between collection name
        ** and open brace.  Note that brace comments are not allowed!
        ** NB: Do not want to look for { ... } with sqlcomment() since
        ** that will track the entire collection LIST{v1, v2, ..., vN},
        ** (unless it contains nested {} comments, of course).
        */
        const char *src = *end;
        const char *c_bgn;
        const char *c_end;
        int   style = JLSS_CSTYLE_COMMENT | JLSS_ISOSQL_COMMENT;
        SQLComment cmt;
        DB_TRACE(0, "----iustoken: <<%.32s%s>>\n", src, (strlen(src) > 32 ? "..." : ""));
        while ((cmt = sqlcomment(src, style, &c_bgn, &c_end)) == SQL_COMMENT)
        {
            src = c_end;
            DB_TRACE(0, "----iustoken: <<%.32s%s>>\n", src, (strlen(src) > 32 ? "..." : ""));
        }
        if (*c_bgn == LCURLY)
            *end = c_bgn + 1;
    }
    DB_TRACE(0, "<<--iustoken: <<%.*s>>\n", *end - token, token);
    return CONST_CAST(char *, token);
}

#ifdef TEST

#include <stdio.h>

#define DIM(x)  (sizeof(x)/sizeof(*(x)))

static char *input[] =
{
    " \t\v\f\n\r ", /* Pure white space (NB: \b backspace is not white space) */
    "{SELECT * FROM SysTables}", /* Pure comment */
    "SELECT * FROM SysTables",
    "SELECT { * } Tabid FROM SysTables",
    "SELECT -- * \n Tabid FROM SysTables",
    "SELECT #-- * \n Tabid FROM SysTables", /* Obsolete # comment */
    "SELECT a+b FROM 'informix'.systables",
    "SELECT a+1 AS\"a\"\"b\",a+1.23AS'a''b2'FROM db@server:\"user\".table\n"
        "WHERE (x+2 UNITS DAY)>=(DATETIME(1998-12-23 13:12:10) YEAR TO SECOND-1 UNITS DAY)\n"
        "  AND t<+3.14159E+32\n",
    "SELECT a.--this should be in comment and invisible\n"
        "b FROM SomeDbase:{this should be in comment and invisible too}\n"
        "user.#--more commentary\n\t\ttablename", /* Obsolete # comment */
    "SELECT (a>=<=<>!=||...(b)) FROM Nowhere",
    "{cc}-1{c}+1{c}.1{c}-.1{c}+.1{}-1.2E3{c}+1.23E+4{c}-1.234e-56{c}-1.234E",
    "info columns for 'cdhdba'.cdh_user",
    "select a::type as _ from _",
    "select {+ hint} _ as _ from _",
    "select --+ hint\n _ as _ from _",
    "select /*+ hint*/ _ as _ from _",
    "select 'abc\ndef' from has_newline",
    /* Multi-line string */
    "INSERT{}INTO{}Foo{} SELECT{}Id{},{}Val_One{},{}Val_Two{}"
        "FROM{}TABLE{}({}SET{{}ROW{}({}1{},{}'Paul'{},{}'Paul'{}){},"
        "{}ROW{}({}2{},{}'Paul '{},{}'Paul '{}){}"
        ",{}ROW{}({}3{},{}'Paul  '{},{}'Paul  '{}){}}"
        "::SET(ROW(Id INTEGER, Val_One CHAR(10), Val_Two VARCHAR(10))"
        "{}NOT{}NULL){})",
    "TABLE{}({}MULTISET{{}ROW{}({}1{},{}'Paul'{},{}'Paul'{}){}})",
    "TABLE{}({}LIST{{}ROW{}({}1{},{}'Paul'{},{}'Paul'{}){}})",
    "TABLE{}({}SET\n\t{{}ROW{}({}1{},{}'Paul'{},{}'Paul'{}){}})",
    "TABLE{}({}MULTISET  \n\n\t{{}ROW{}({}1{},{}'Paul'{},{}'Paul'{}){}})",
    "TABLE{}({}LIST--comment\n{{}SET{{}1{},{}2{},{}3{}}{}}){}\t\n",
    "TABLE{}({}LIST/*comment*/{{}SET{{}1{},{}2{},{}3{}}{}}){}\t\n",
};

int main(void)
{
    int i;
    int n;
    const char *str;
    const char *src;
    const char *end;
    char  buffer[2048];

    for (i = 0; i < DIM(input); i++)
    {
        n = 0;
        str = input[i];
        printf("Data: <<%s>>\n", str);
        while (*(src = iustoken(str, &end)) != '\0' && src != end)
        {
            strncpy(buffer, src, end - src);
            buffer[end - src] = '\0';
            n++;
            printf("Token %d: <<%s>>\n", n, buffer);
            str = end;
        }
        if (n == 0)
            printf("== No tokens found ==\n");
    }
    printf("** TEST COMPLETE **\n");
    return 0;
}

#endif /* TEST */

