/*
@(#)File:           $RCSfile: guesser.c,v $
@(#)Version:        $Revision: 1.10 $
@(#)Last changed:   $Date: 2013/09/08 22:30:16 $
@(#)Purpose:        Guess whether to update or insert, based on recent actions
@(#)Author:         J Leffler
@(#)Copyright:      (C) JLSS 1998,2000-01,2003,2005
@(#)Product:        SQLCMD Version 90.02 (2016-07-28)
*/

/*TABSTOP=4*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <ctype.h>
#include <stdio.h>
#include "upload.h"
#include "stderr.h"

#define DIM(x)  (sizeof(x)/sizeof(*(x)))

static HMode actions[99];
static int  act_num = DIM(actions);
static int  act_ptr = 0;
static int  ucount = 0;
static int  icount = 0;
static HMode h_mode = H_HEURISTIC;

#ifndef lint
/* Prevent over-aggressive optimizers from eliminating ID string */
const char jlss_id_guesser_c[] = "@(#)$Id: guesser.c,v 1.10 2013/09/08 22:30:16 jleffler Exp $";
#endif /* lint */

/* Configure and reset heuristics */
/* Could dynamically allocate space, if desired */
void set_heuristic(HMode mode, size_t size)
{
    int i;

    h_mode = mode;

    act_num = size;
    if (size <= 0 || size > DIM(actions))
        act_num = DIM(actions);
    for (i = 0; i < act_num; i++)
        actions[i] = H_UNDEFINED;
}

/* Get best guess as to what to do */
HMode get_heuristic(void)
{
    HMode h = H_UNDEFINED;
    switch (h_mode)
    {
    case H_HEURISTIC:
        h = (icount > ucount) ? H_INSERT : H_UPDATE;
        break;
    case H_INSERT:
        h = H_INSERT;
        break;
    case H_UPDATE:
    case H_UNDEFINED:
        h = H_UPDATE;
        break;
    case H_SELECT:
        h = H_SELECT;
        break;
    default:
        /* h_mode should not contain any of the other values in HMode */
        err_abort("%s:%d: Heuristic mode out of control\n", __FILE__, __LINE__);
        break;
    }
    return h;
}

/* Update guessing system with new information */
void upd_heuristic(char c)
{
    HMode m = char_to_HMode(c);

    if (actions[act_ptr] == H_INSERT)
        icount--;
    else if (actions[act_ptr] == H_UPDATE)
        ucount--;
    actions[act_ptr] = m;
    if (m == H_INSERT)
        icount++;
    else if (m == H_UPDATE)
        ucount++;
    act_ptr = (act_ptr + 1) % act_num;
}

/* Print state of guessing system on specified file */
/* Prints one line of data only */
void prt_heuristic(FILE *fp)
{
    int i;

    fprintf(fp, "icount = %2d; ucount = %2d; ", icount, ucount);
    for (i = 0; i < act_num; i++)
    {
        if (actions[i] == H_UNDEFINED)
            putc('-', fp);
        else
            putc(HMode_to_char(actions[i]), fp);
    }
    putc('\n', fp);
}

HMode char_to_HMode(char c)
{
    HMode h;

    c = toupper(c);
    switch (c)
    {
    case 'H':
        h = H_HEURISTIC;
        break;
    case 'I':
        h = H_INSERT;
        break;
    case 'U':
        h = H_UPDATE;
        break;
    case 'S':
        h = H_SELECT;
        break;
    default:
        h = H_UNDEFINED;
        break;
    }
    return(h);
}

char HMode_to_char(HMode m)
{
    return((char)m);
}

#ifdef TEST

int main(int argc, char **argv)
{
    char actual;
    char line[80];
    int ccount = 0;
    char *src;
    char guess;
    int mcount = 0;
    int utotal = 0;
    int itotal = 0;
    int verbose = 0;

    verbose = argc - 1;

    set_heuristic(H_HEURISTIC, 0);      /* More intelligent test would use an argument */

    while (fgets(line, sizeof(line), stdin) != 0)
    {
        src = line;
        while ((actual = *src++) != '\0')
        {
            if (actual == 'u' || actual == 'i')
            {
                ccount++;
                guess = get_heuristic();
                if (guess == actual)
                    mcount++;
                upd_heuristic(actual);
                if (actual == 'u')
                    utotal++;
                else
                    itotal++;
                if (verbose)
                {
                    printf("%3d: Guess = %c Actual = %c (%c): ", ccount, guess, actual, (actual == guess) ? '+' : '-');
                    if (verbose > 1)
                        prt_heuristic(stdout);
                    else
                        putchar('\n');
                }
            }
        }
    }

    printf("Totals: update = %3d, insert = %3d, match = %3d (%3.1f%%)\n", utotal, itotal, mcount, (mcount * 100.0)/(utotal + itotal));

    return 0;
}

#endif /* TEST */
