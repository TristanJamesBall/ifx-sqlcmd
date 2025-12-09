/*
@(#)File:           $RCSfile: getline.c,v $
@(#)Version:        $Revision: 3.3 $
@(#)Last changed:   $Date: 2015/06/02 04:47:55 $
@(#)Purpose:        Read line from file
@(#)Author:         J Leffler
@(#)Copyright:      (C) JLSS 1987,1991,1997,1999-2001,2005,2008,2010,2015
@(#)Product:        SQLCMD Version 90.02 (2016-07-28)
*/

/*TABSTOP=4*/

#include "jlss.h"
#include "kludge.h"
#include <assert.h>

#ifndef lint
/* Prevent over-aggressive optimizers from eliminating ID string */
extern const char jlss_id_getline_c[];
const char jlss_id_getline_c[] = "@(#)$Id: getline.c,v 3.3 2015/06/02 04:47:55 jleffler Exp $";
#endif /* lint */

/*
** -- Routine: jlss_getline
**
** Read a line of data into a buffer of given size, and terminate
** the line with an ASCII NUL '\0'.  Returns the number of bytes
** of data processed (excluding the NUL).
**
** There are two compile-time options:
** -DDISCARD_NEWLINE: discards the newline.
** -DDISCARD_RESIDUE: discards any surplus characters in each line.
**     There is no reliable indication that data has been discarded.
**
** The return values are:
**      EOF: no more data (EOF); string is empty.
**      >=0: N characters were read.
**
** The main difference between the default version of this function and
** fgets() is the return value -- fgets() does not tell you how many
** characters were read, even though that would be very useful.
**
** Note that fgets() can be more efficient if it exploits the internals
** of the FILE buffer structures.
*/
int jlss_getline(FILE *fp, char *buffer, size_t buflen)
{
    int             c = 0;  /* Unnecessary, but keeps GCC quiet */
    size_t          i;

    assert(buflen != 0);
    assert(buffer != 0);

    /* Read as much of line as possible */
    for (i = 0; --buflen > 0 && (c = getc(fp)) != EOF && c != '\n'; i++)
        buffer[i] = (char)c;

#ifdef DISCARD_NEWLINE
    FEATURE("jlss_getline(): DISCARD_NEWLINE");
#else
    FEATURE("jlss_getline(): RETAIN_NEWLINE");
    /* Tack newline onto end */
    if (buflen > 0 && c == '\n')
        buffer[i++] = (char)c;
#endif /* DISCARD_NEWLINE */

#ifdef DISCARD_RESIDUE
    FEATURE("jlss_getline(): DISCARD_RESIDUE");
    /* Read and discard rest of line */
    if (buflen <= 0 && c != '\n')
    {
        while ((c = getc(fp)) != EOF && c != '\n')
            ;
    }
#else
    FEATURE("jlss_getline(): RETAIN_RESIDUE");
#endif /* DISCARD_RESIDUE */

    /* Null terminate string */
    buffer[i] = '\0';

    /* Return length of string */
    return((i == 0 && c == EOF) ? EOF : (int)i);
}

#if defined(TEST)

int main(void)
{
    int             len;
    char            buffer[20];

    printf("Enter line (buflen %lu): ", (unsigned long)sizeof(buffer));
    while ((len = jlss_getline(stdin, buffer, sizeof(buffer))) != EOF)
    {
        printf("Read %d: <<<%s>>>\n", len, buffer);
        printf("Enter line: ");
    }
    return(0);
}

#endif /* TEST */
