/*
@(#)File:           $RCSfile: mkstemp.c,v $
@(#)Version:        $Revision: 1.3 $
@(#)Last changed:   $Date: 2015/10/25 22:36:15 $
@(#)Purpose:        Surrogate for mkstemp()
@(#)Author:         J Leffler
@(#)Copyright:      (C) JLSS 2008,2011,2015
@(#)Product:        SQLCMD Version 90.02 (2016-07-28)
*/

/*TABSTOP=4*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include "posixver.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "openfile.h"

#ifndef HAVE_MKSTEMP
extern int mkstemp(char *namestr);
#endif /* HAVE_MKSTEMP */

/*
** mkstemp - make a unique filename
**
** #include <stdlib.h>
** int mkstemp(char *template);
**
** The mkstemp() function shall replace the contents of the string
** pointed to by template by a unique filename, and return a file
** descriptor for the file open for reading and writing.  The function
** thus prevents any possible race condition between testing whether the
** file exists and opening it for use.  The string in template should
** look like a filename with six trailing 'X' s; mkstemp() replaces each
** 'X' with a character from the portable filename character set.  The
** characters are chosen such that the resulting name does not duplicate
** the name of an existing file at the time of a call to mkstemp().
**
** Upon successful completion, mkstemp() shall return an open file
** descriptor.  Otherwise, -1 shall be returned if no suitable file
** could be created. 
*/

/*
** Implementation information.
**
** configure.ac guideline:
** AC_CHECK_FUNCS([mkstemp], [break])
**
** Optional alternative:
** AC_CHECK_FUNCS([mkstemp tempnam tmpnam mktemp], [break])
**
** This stops on the first of the functions that is found.
**
** Problems with these fallbacks:
** char *tempnam(const char *dir, const char *pfx);
**     Not using XXXXXX template - tempnam() might call tmpnam().
** char *tmpnam(char *s);
**     No control over location
** char *mktemp(char *template);
**     LEGACY - not going to be there forever.
**     Linux warns about programs that use this.
**
** If an implementation does not support mkstemp(), the chances are
** rather high that it does not support tempnam() either.  Consequently,
** this code does not provide a fall-back using any of the alternatives,
** not least because the objective is to avoid warnings about using
** mktemp().
**
** The code uses open_tmpfile() from JLSS openfile.c to safely create
** the temporary file (O_CREAT|O_EXCL|O_RDWR).  The permissions on the
** file are 664.
*/

#ifndef lint
/* Prevent over-aggressive optimizers from eliminating ID string */
extern const char jlss_id_mkstemp_c[];
const char jlss_id_mkstemp_c[] = "@(#)$Id: mkstemp.c,v 1.3 2015/10/25 22:36:15 jleffler Exp $";
#endif /* lint */

#ifndef HAVE_MKSTEMP

#ifndef DEFAULT_TMPDIR
#define DEFAULT_TMPDIR "/tmp"
#endif /* DEFAULT_TMPDIR */

/*
** Note: ISO C says getenv() might overwrite previous return values on
** multiple calls.  Do not take the risk.
*/

/* Return a suitable directory as the default directory */
/* Returns null on memory allocation error */
static const char *gen_def_tmpdir(void)
{
    static char *def_tmpdir = 0;

    if (def_tmpdir == 0)
    {
        const char *name;
        size_t len;
        if (((name = getenv("DBTEMP")) == 0) &&
            ((name = getenv("TMPDIR")) == 0))
            name = DEFAULT_TMPDIR;
        len = strlen(name) + 1;
        if ((def_tmpdir = (char *)malloc(len)) != 0)
            strcpy(def_tmpdir, name);
    }
    return(def_tmpdir);
}

/* Create a suitable default file template */
/* Returns null on memory allocation error */
static const char *gen_def_template(void)
{
    static const char def_name[] = "mkstemp.XXXXXX";
    static char *def_template = 0;

    if (def_template == 0)
    {
        const char *tmpdir = gen_def_tmpdir();
        if (tmpdir != 0)
        {
            size_t len = strlen(tmpdir) + strlen(def_name) + sizeof("/");
            if ((def_template = (char *)malloc(len)) != 0)
            {
                /* This is already length-checked! */
                if (snprintf(def_template, len, "%s/%s", tmpdir, def_name) != len - 1)
                {
                    free(def_template);
                    def_template = 0;
                }
            }
        }
    }
    return(def_template);
}

/* Generate temporary file name.
** Sequence of names (6 * 36), assuming PID is 12345.
**     12345a, ..., 12345z, 123450, ..., 123459,
**     1234a5, ..., 1234z5, 123405, ..., 123495,
**     123a45, ..., 123z45, 123045, ..., 123945,
**     12a345, ..., 12z345, 120345, ..., 129345,
**     1a2345, ..., 1z2345, 102345, ..., 192345,
**     a12345, ..., z12345, 012345, ..., 912345,
** Take conservative, case-sensitive view of life (hence no upper-case,
** no punctuation, but do allow digits in lieu of letter).
** Take less conservative view - temp files are removed quickly.
** Hence, OK to start over when sequence runs out.
** OK in this context; the files are removed after use.
** Leaves open a DOS - generate all 216 files in advance...
** However, that is a problem for the calling code, not this function.
*/

static const char fillins[] = "abcdefghijklmnopqrstuvwxyz0123456789";
#ifdef TEST_NAME_CYCLING
enum { MAX_OFFSET = 3 };    /* Fast cycling names */
#else
enum { MAX_OFFSET = sizeof(fillins) - 1 };
#endif /* TEST_NAME_CYCLING */
enum { MAX_TMPNAMES = MAX_OFFSET * 6 };

/* Need to recognize $TMPDIR - and handle longer names, therefore */
static const char *gentmpname(void)
{
    static int  tmp_init = 0;
    static int  tmp_pid;
    static const char *tmp_ltr = fillins;
    static int  tmp_idx = 5;
    static char tmp_str[7];

    /* Reset when last letter already generated */
    if (++tmp_ltr > fillins + MAX_OFFSET && tmp_idx <= 0)
        tmp_init = 0;

    if (tmp_init == 0)
    {
        tmp_pid = getpid() % 100000;
        tmp_idx = 5;
        tmp_ltr = fillins;
        /* Safe - length checked! */
        sprintf(tmp_str, "%05d%c", tmp_pid, *tmp_ltr);
        tmp_init = 1;
    }
    else if (tmp_ltr > fillins + MAX_OFFSET)
    {
        char t = tmp_str[tmp_idx];
        tmp_str[tmp_idx] = tmp_str[tmp_idx-1];
        tmp_str[--tmp_idx] = t;
        tmp_ltr = fillins;
    }

    tmp_str[tmp_idx] = *tmp_ltr;

    return(tmp_str);
}

/* Renamed argument from template - to permit C++ compilation */

int mkstemp(char *namestr)
{
    int fd;
    char buffer[256];
    char *name = buffer;
    const char *refname = namestr;
    size_t len = strlen(refname) + 1;
    char *suffix;
    size_t i;

    if (len < 7 || strcmp(refname + len - sizeof("XXXXXX"), "XXXXXX") != 0)
    {
        /* The user screwed up - cover for his mistake! */
        refname = gen_def_template();
        if (refname == 0)
            return(-1);
        len = strlen(refname) + 1;
    }

    if (len > sizeof(buffer))
    {
        /* User-supplied name is longer than buffer - do not overflow! */
        name = (char *)malloc(len);     /*=C++=*/
        if (name == 0)
            return(-1);
    }

    /* Safe - length-checked */
    strcpy(name, refname); 
    suffix = name + len - sizeof("XXXXXX");
    for (i = 0; i < MAX_TMPNAMES; i++)
    {
        strcpy(suffix, gentmpname());
        if ((fd = open_tmpfile(name)) != -1)
            break;
    }

    if (fd != -1)
        strcpy(namestr, name);

    if (name != buffer)
        free(name);

    return(fd);
}

#endif /* HAVE_MKSTEMP */

#ifdef TEST

#ifdef HAVE_MKSTEMP
enum { MAX_TMPNAMES = TMP_MAX };
#endif /* HAVE_MKSTEMP */

#include <stdio.h>
#include "stderr.h"

static const char mytmp[] = "/tmp/mkstest.XXXXXX";

/* Basic test of mkstemp() */
static void test_mkstemp(char *buffer, size_t i)
{
    /* Safe copy - at the moment */
    int fd = mkstemp(buffer);
    if (fd < 0)
        err_syserr("Failed to create temp file number %zu\n", i);
    if (close(fd) != 0)
        err_syserr("Failed to close temp file %zu (%s)\n", i, buffer);
    if (unlink(buffer) != 0)
        err_syserr("Failed to unlink temp file %zu (%s)\n", i, buffer);
    if (i % 64 == 0)
        puts(buffer);
    putchar('.');
    if ((i + 1) % 64 == 0)
        putchar('\n');
}

/* Test what happens with no cleanup */
static void no_remove_test(void)
{
    size_t i;
    char    *list[256];
    size_t   list_size = 0;

    for (i = 0; i < MAX_TMPNAMES + 3; i++)
    {
        char buffer[256];
        strcpy(buffer, mytmp);
        /* Safe copy - at the moment */
        int fd = mkstemp(buffer);
        if (fd < 0)
            err_sysrem("Failed to create temp file number %zu\n", i);
        else
        {
            if (close(fd) != 0)
                err_syserr("Failed to close temp file %zu (%s)\n", i, buffer);
            list[list_size++] = strdup(buffer);
        }
        if (i % 64 == 0)
            puts(buffer);
        putchar('.');
        if ((i + 1) % 64 == 0)
            putchar('\n');
    }
    if (i % 64 != 0)
        putchar('\n');

    for (i = 0; i < list_size; i++)
    {
        if (unlink(list[i]) != 0)
            err_sysrem("Failed to unlink temp file %zu (%s)\n", i, list[i]);
        putchar('.');
        if ((i + 1) % 64 == 0)
            putchar('\n');
    }
    if (i % 64 != 0)
        putchar('\n');
}

int main(int argc, char **argv)
{
    size_t i;

    err_setarg0(argv[argc-argc]); /* Use argc */

    /* Basic testing */
    for (i = 0; i < MAX_TMPNAMES; i++)
    {
        char buffer[256];
        /* Safe copy - at the moment */
        strcpy(buffer, mytmp);
        test_mkstemp(buffer, i);
    }
    if (i % 64 != 0)
        putchar('\n');

    /* Test what happens with erroneous templates */
    for (i = 0; i < MAX_TMPNAMES; i++)
    {
        char buffer[256];
        if (i % 10 == 0)
            strcpy(buffer, mytmp);
        test_mkstemp(buffer, i);
    }
    if (i % 64 != 0)
        putchar('\n');

    /* Test what happens with no cleanup */
    no_remove_test();

    return(0);
}

#endif /* TEST */
