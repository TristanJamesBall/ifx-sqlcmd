/*
@(#)File:           $RCSfile: strdotfill.c,v $
@(#)Version:        $Revision: 1.14 $
@(#)Last changed:   $Date: 2015/05/25 06:31:26 $
@(#)Purpose:        Safely copy long string to short buffer
@(#)Author:         J Leffler
@(#)Copyright:      (C) JLSS 2004-08,2011,2015
@(#)Product:        SQLCMD Version 90.02 (2016-07-28)
*/

/*TABSTOP=4*/

#include "strdotfill.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "ilog10.h"

static inline size_t MIN(size_t x, size_t y) { return(((x) < (y)) ? (x) : (y)); }

#ifndef lint
/* Prevent over-aggressive optimizers from eliminating ID string */
extern const char jlss_id_strdotfill_c[];
const char jlss_id_strdotfill_c[] = "@(#)$Id: strdotfill.c,v 1.14 2015/05/25 06:31:26 jleffler Exp $";
#endif /* lint */

/* This code should be safe even if assert is disabled via #define NDEBUG. */

/*
** strdotomit - when string is too long to fit in buffer, copy leading
** and trailing characters, documenting the number of omitted bytes.
** If there isn't enough space in the output, fall back on strdotfill().
** If trail is too long, balance leading and trailing sections.
** If the string is bigger than the buffer, the output will use the whole buffer
** or the buffer minus one byte.
*/
char *strdotomit(char *buffer, size_t buf_len, const char *str, size_t str_len, size_t trail)
{
    assert(buffer != 0 && buf_len > 0 && str != 0);
    if (buffer == 0 || buf_len == 0)
        return(buffer);
    else if (str == 0)
    {
        buffer[0] = '\0';
        return(buffer);
    }
    else if (str_len < buf_len)
    {
        /* Don't trust the value of str_len! */
        size_t copy_len = MIN(buf_len - 1, str_len);
        strncpy(buffer, str, copy_len);
        buffer[copy_len] = '\0';
    }
    else
    {
        char fill_buffer[32];   /* Plenty long enough for "[...987654321 bytes...]" */
        size_t ndigits = ilog10_z(str_len) + 1;
        size_t fill_len = ndigits + sizeof("[... bytes...]") - 1;
        if (fill_len + 3 > buf_len)
        {
            /* Buffer is too short for "a[...123 bytes...]z" */
            strdotfill(buffer, buf_len, str, str_len);
        }
        else
        {
            size_t omitted = str_len - (buf_len - 1 - fill_len);

            if (trail >= buf_len - fill_len - 1)
                trail = (buf_len - fill_len) / 2;

            size_t lead = buf_len - 1 - fill_len - trail;
            int nbytes = sprintf(fill_buffer, "[...%d bytes...]", (int)omitted);
            assert(nbytes > 0);
            size_t nb = (size_t)nbytes;

            if (nb != fill_len)
            {
                assert(nb == fill_len - 1);
                omitted--;
                sprintf(fill_buffer, "[...%d bytes...]", (int)omitted);
                lead++;
            }

            sprintf(buffer, "%.*s%s%.*s", (int)lead, str, fill_buffer,
                    (int)trail, &str[str_len - trail]);
        }
    }
    return(buffer);
}

/*
** strdotfill - when string is too long to fit in buffer, copy leading
** characters followed by 3 trailing dots.
*/
char *strdotfill(char *buffer, size_t buf_len, const char *str, size_t str_len)
{
    assert(buffer != 0 && buf_len > 0 && str != 0);
    if (buffer == 0 || buf_len == 0)
        return(buffer);
    else if (str == 0)
    {
        buffer[0] = '\0';
        return(buffer);
    }
    else if (str_len < buf_len)
    {
        /* Don't trust the value of str_len! */
        size_t copy_len = MIN(buf_len - 1, str_len);
        strncpy(buffer, str, copy_len);
        buffer[copy_len] = '\0';
    }
    else
    {
        const char *fill = "...";
        size_t wlen = buf_len - sizeof("...");

        if (buf_len < sizeof("X..."))
        {
            /* Ridiculously short buffer */
            wlen = buf_len - 1;
            str  = fill;
            fill = "";
        }
        sprintf(buffer, "%.*s%s", (int)wlen, str, fill);
    }
    return(buffer);
}

