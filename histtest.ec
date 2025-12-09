/*
@(#)File:           $RCSfile: histtest.ec,v $
@(#)Version:        $Revision: 2.19 $
@(#)Last changed:   $Date: 2009/03/30 02:01:42 $
@(#)Purpose:        Test Program for the HISTORY Package in SQLCMD
@(#)Author:         J Leffler
@(#)Copyright:      (C) JLSS 1992,2001,2003,2005,2008-09
@(#)Product:        SQLCMD Version 90.02 (2016-07-28)
*/

/*TABSTOP=4*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "jlsstools.h"
#include "esnprintf.h"
#include "context.h"
#include "emalloc.h"
#include "history.h"
#include "sqlcmd.h"
#include "stderr.h"
#include "readcmd.h"

#ifndef DEF_EDITOR
#define DEF_EDITOR "vi"
#endif /* DEF_EDITOR */

#define TMPFILE_TEMPLATE "/tmp/histtest.XXXXXX"

int pflag = SQLUNLOAD;

static void read_file(FILE *fp);

#ifndef lint
/* Prevent over-aggressive optimizers from eliminating ID string */
const char jlss_id_histtest_ec[] = "@(#)$Id: histtest.ec,v 2.19 2009/03/30 02:01:42 jleffler Exp $";
#endif /* lint */

/* Create and open a named temporary file */
/* NB: copies opened name into fname variable! */
static FILE *fopen_namedtmpfile(char *fname, size_t bufsiz)
{
    FILE *fp = 0;

    if (bufsiz < sizeof(TMPFILE_TEMPLATE))
        err_error("Configuration error: editor file name buffer too short");
    strcpy(fname, TMPFILE_TEMPLATE);
    fp = fmkstemp(fname);
    return fp;
}

static void do_erase(char *buffer)
{
    long     c1;
    long     c2;

    fflush(stdout);
    c1 = c2 = 0;
    if (sscanf(&buffer[sizeof("@erase") - 1], "%ld%ld", &c1, &c2) != 2)
        c2 = c1;
    fprintf(stderr, "Erase: %ld..%ld\n", c1, c2);
    hist_erase(c1, c2);
}

static void do_rerun(char *buffer)
{
    long     c1;
    long     c2;
    FILE    *fp;

    fflush(stdout);
    c1 = c2 = 0;
    if (sscanf(&buffer[sizeof("@rerun") - 1], "%ld%ld", &c1, &c2) != 2)
        c2 = c1;
    fprintf(stderr, "Rerun: %ld..%ld\n", c1, c2);
    fp = tmpfile();
    hist_output(fp, c1, c2, H_COMMAND);
    fseek(fp, 0L, SEEK_SET);
    if ((c1 = hist_input(fp)) != 0)
        cmd_set_promptnum(c1);
    fseek(fp, 0L, SEEK_SET);
    ctxt_newcontext();
    ctxt_newinput(fp, "<<temp>>");
    ctxt_sethistory("off");
    read_file(ctxt_input());
    ctxt_endcontext();
    fclose(fp);
}

static void do_edit(char *buffer)
{
    FILE           *fp;
    long            c1;
    long            c2;
    char            sys[BUFSIZ];
    char            fname[BUFSIZ];
    const char     *editor;

    fflush(stdout);
    c1 = c2 = 0;
    if (sscanf(&buffer[sizeof("@edit") - 1], "%ld%ld", &c1, &c2) != 2)
        c2 = c1;
    fprintf(stderr, "Edit: %ld..%ld\n", c1, c2);
    if ((fp = fopen_namedtmpfile(fname, sizeof(fname))) == (FILE *)0)
        err_error("cannot open editing file %s for writing\n", fname);
    hist_output(fp, c1, c2, H_COMMAND);
    fclose(fp);

    if ((editor = getenv("DBEDIT")) == NIL(char *) &&
        (editor = getenv("VISUAL")) == NIL(char *) &&
        (editor = getenv("EDITOR")) == NIL(char *))
        editor = DEF_EDITOR;
    if (strlen(editor) + strlen(fname) + 2 >= sizeof(sys))
        err_error("editor name (%s) + file name (%s) too long\n", editor, fname);
    esnprintf(sys, sizeof(sys), "%s %s", editor, fname);
    if (system(sys) != 0)
        err_syserr("problem executing command (%s)\n", sys);

    if ((fp = fopen(fname, "r")) == (FILE *)0)
        err_error("cannot open editing file %s for reading\n", fname);
    if ((c1 = hist_input(fp)) != 0)
        cmd_set_promptnum(c1);
    fseek(fp, 0L, SEEK_SET);
    ctxt_newcontext();
    ctxt_newinput(fp, "<<temp>>");
    ctxt_sethistory("off");
    read_file(ctxt_input());
    ctxt_endcontext();
    if (unlink(fname) != 0)
        err_syserr("failed to unlink temp file %s\n", fname);
}

static void do_list(char *buffer)
{
    long    c1;
    long    c2;

    c1 = c2 = 0;
    if (sscanf(&buffer[sizeof("@list") - 1], "%ld%ld", &c1, &c2) != 2)
        c2 = c1;
    fflush(stdout);
    fprintf(stderr, "List: %ld..%ld\n", c1, c2);
    hist_output(stdout, c1, c2, H_NUMBERS);
}

static void do_adjust(char *buffer)
{
    long            c2;

    c2 = 0;
    if (sscanf(&buffer[sizeof("@adjust") - 1], "%ld", &c2) == 1)
    {
        fflush(stdout);
        fprintf(stderr, "Adjust: %ld\n", c2);
        hist_setsize(c2);
    }
}

static void do_help(void)
{
    static const char helpstr[] =
        "HISTTEST: Test History Handling\n"
        "History commands (start with @):\n"
        "  @adjust N    - set history size to N\n"
        "  @dump        - print data about commands\n"
        "  @edit N [M]  - edit commands N [to M] and 'rerun'\n"
        "  @erase N [M] - erase commands N [to M]\n"
        "  @exit        - terminate\n"
        "  @help        - print this help\n"
        "  @list N [M]  - list history size to N\n"
        "  @rerun N [M] - rerun commands N [to M]\n"
        "Anything else is treated as a command to be recorded and executed.\n"
        ;
    fputs(helpstr, stdout);
}

static void read_file(FILE *fp)
{
    char            buffer[BUFSIZ];

    while (cmd_read(fp, buffer, sizeof(buffer)) != EOF)
    {
        if (buffer[0] == '\0')
            continue;
        else if (buffer[0] == '@')
        {
            if (strncmp(buffer, "@dump", sizeof("@dump") - 1) == 0)
                hist_dump(stderr);
            else if (strncmp(buffer, "@adjust", sizeof("@adjust") - 1) == 0)
                do_adjust(buffer);
            else if (strncmp(buffer, "@list", sizeof("@list") - 1) == 0)
                do_list(buffer);
            else if (strncmp(buffer, "@edit", sizeof("@edit") - 1) == 0)
                do_edit(buffer);
            else if (strncmp(buffer, "@rerun", sizeof("@rerun") - 1) == 0)
                do_rerun(buffer);
            else if (strncmp(buffer, "@erase", sizeof("@erase") - 1) == 0)
                do_erase(buffer);
            else if (strncmp(buffer, "@help", sizeof("@help") - 1) == 0)
                do_help();
            else if (strncmp(buffer, "@exit", sizeof("@exit") - 1) == 0)
                break;
            else
                err_remark("command not understood: %s\n", buffer);
        }
        else
        {
            printf("Command: %s\n", buffer);
            if (ctxt_gethistory() == OP_ON)
                cmd_set_promptnum(hist_write(buffer));
        }
    }
}

int main(int argc, char **argv)
{
    long        cmdnum;

    err_setarg0(argv[0]);
    ctxt_init();
    ctxt_sethistory("on");
    do_help();
    if ((cmdnum = hist_open(H_MODIFY)) >= 0)
    {
        cmd_set_promptnum(cmdnum);
        read_file(stdin);
        hist_close();
    }
    else
        err_error("history file open error %ld\n", cmdnum);
    ctxt_free();
#ifdef DEBUG_MALLOC
    dump_malloc();
#endif  /* DEBUG_MALLOC */
    return(0);
}

void sql_longjmp(int status)
{
    err_error("arrived in sql_longjmp!!\n");
}
