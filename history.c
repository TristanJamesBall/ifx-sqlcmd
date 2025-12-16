/*
@(#)File:           $RCSfile: history.c,v $
@(#)Version:        $Revision: 2015.1 $
@(#)Last changed:   $Date: 2015/07/10 15:43:53 $
@(#)Purpose:        History handling for SQLCMD
@(#)Author:         J Leffler
@(#)Copyright:      (C) JLSS 1992-93,1996-98,2000-03,2005-06,2008-10,2015
@(#)Product:        SQLCMD Version 90.02 (2016-07-28)
*/

/*TABSTOP=4*/

/*
**  The history file has the following structure:
**  * Magic Number           (4 bytes)   SQLCMD_MAGIC2
**  * History Size           (4 bytes)   Number of entries in lists
**  * Next Entry             (4 bytes)   Next entry to overwrite
**  * History sequence       (4 bytes)   Sequentially increasing entry number
**  * Size of header section (4 bytes)
**  * Number of free entries (4 bytes)
**  * Nominal end of file    (4 bytes)
**  * Active Entry List      (8 bytes per entry)
**  * Free Entry List        (8 bytes per entry)
**  * End of Header          (4 bytes)   0x0A0A0A0A
**  Both active entries and free entries have the structure
**  * Offset (4 bytes) measured from end of header
**  * Length (4 bytes) including terminal newline
**
**  Starting with history.c version 2.1, the sizes are machine
**  independent (previously, the sizes varied horribly depending on
**  32-bit or 64-bit architectures), and are written/read using the
**  ld_sint4() and st_sint4() functions (so they are platform
**  independent).  Because of these changes, the magic number for
**  history files was changed.  There is no provision to upgrade
**  the history file.
**
**  The following primitives are provided to maintain the history file:
**  * hist_file    -- set and return the name of the history file
**  * hist_open    -- opens or creates a history file
**  * hist_close   -- closes a history file
**  * hist_write   -- writes a new entry to the history file
**  * hist_read    -- reads a previous entry from the history file
**  * hist_setsize -- changes the size (capacity) of the history file
**  * hist_getsize -- returns the current size (capacity) of the history file
**  * hist_dump    -- print internal diagnostic information
**  * hist_output  -- print a series of history records (optionally numbered)
**  * hist_input   -- read a file as a single command
**  * hist_erase   -- erase previous history entries
**
********************************************************
**  Implementation Notes
**  1.  The free list is kept with the free slots in sorted order.
**  2.  Any free space after the highest used entry is ignored.
**  3.  => There are no more free entries than there are used entries
**  4.  At worst, the file consists of free, used, free, used, ... entries
**  5.  The value of h_head.eof is the offset of the first byte after
**      the highest used entry.
**  6.  The current value of one used slot is freed before a new entry
**      is written.
**  7.  When the freed slot is the highest slot, the highest free slot
**      needs to be ignored if it is contiguous with the freed slot.
**  8.  When a slot is allocated, the first free slot which is big
**      enough to take it is used.
**  9.  If the slot is exactly big enough, all the other free slots
**      have to be shuffled down one entry.
**  10. The strings in the file are not null terminated.  They are
**      usually terminated with a newline.
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include "posixver.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "filelock.h"
#include "emalloc.h"
#include "history.h"
#include "sqlcmd.h"
#include "jlsstools.h"
#include "stderr.h"

#define SQLCMD_MAGIC1   0x27160DA4  /* old magic - history.[ch] v1.x */
#define SQLCMD_MAGIC2   0xD8E9F25B  /* new magic - history.[ch] v2.x */
#define SQLCMD_TRAILSTR "\n\n\n\n"  /* Four newlines */
#define SQLCMD_HISTFILE ".sqlcmdlog"
#define SQLCMD_HISTENV  "SQLCMDLOG"
#define SQLCMD_SIZE     50

#undef MAX  /* 2005-04-06: HP-UX 11i - sys/param.h */
#define MAX(a, b)   (((a) > (b)) ? (a) : (b))
#undef MIN  /* 2005-04-06: HP-UX 11i - sys/param.h */
#define MIN(a, b)   (((a) < (b)) ? (a) : (b))
#define OFFSET(n)   ((2 * sizeof(DiskEntry) * (n)) + sizeof(DiskHeader) + sizeof(SQLCMD_TRAILSTR) - 1)

/* Image of Header Record on Disk */
typedef struct DiskHeader
{
    char            magic[4];       /* Magic number */
    char            size[4];        /* Max number of history entries */
    char            next[4];        /* Next used entry to use */
    char            count[4];       /* Number of entries ever used */
    char            used[4];        /* Number of used entries */
    char            free[4];        /* Number of free entries */
    char            offset[4];      /* Offset of start of entries */
    char            eof[4];         /* Nominal offset of end of file */
}               DiskHeader;

typedef struct Header
{
    Uint4           magic;      /* Magic number */
    Uint4           size;       /* Max number of history entries */
    Uint4           next;       /* Next used entry to use */
    Uint4           count;      /* Number of entries ever used */
    Uint4           used;       /* Number of used entries */
    Uint4           free;       /* Number of free entries */
    Uint4           offset;     /* Offset of start of entries */
    Uint4           eof;        /* Nominal offset of end of file */
}               Header;

/* Image of Entry Record on Disk */
typedef struct DiskEntry
{
    char          offset[4];
    char          length[4];
}               DiskEntry;

typedef struct Entry
{
    Uint4         offset;
    Uint4         length;
}               Entry;

static FILE    *h_file;
static Header   h_head;
static Entry   *h_used;
static Entry   *h_free;
static const char *h_name;
static LockType h_lock = FLK_WRITELOCK; /* FLK_READLOCK if history is read only */

static const char h_lock_error[] = "failed to lock history file in %s()\n";
static const char h_unlk_error[] = "failed to unlock history file in %s()\n";
static const char h_read_error[] = "failed to read history control data in %s()\n";

static const char h_readfail[] = "fread() failed on history data in %s()\n";
static const char h_seekfail[] = "fseek() failed on history data in %s()\n";
static const char h_writefail[] = "fwrite() failed on history data in %s()\n";

#ifndef lint
/* Prevent over-aggressive optimizers from eliminating ID string */
const char jlss_id_history_c[] = "@(#)$Id: history.c,v 2015.1 2015/07/10 15:43:53 jleffler Exp $";
#endif /* lint */

/* Check whether history is initialized: return 1 if it is, 0 if it is not */
int hist_chkinit(void)
{
    return(h_file != 0);
}

/* Free history slot described by offset and length */
static void     hist_free(Uint4 offset, Uint4 length)
{
    Uint4           i;
    Uint4           slot_end;
    Entry          *ep;

    if (offset == 0 && length == 0)
        return;
    if (offset + length == h_head.eof)
    {
        /* Freed slot is highest in file */
        ep = &h_free[h_head.free - 1];
        if (ep->offset + ep->length == offset)
        {
            h_head.free--;
            h_head.eof = ep->offset;
        }
        else
            h_head.eof -= length;
        return;
    }
    /* Freed slot may be contiguous with one or two existing free slots,   */
    /* in which case, we coalesce the new entry with the existing entries. */
    slot_end = offset + length;
    for (i = 0; i < h_head.free; i++)
    {
        if (h_free[i].offset + h_free[i].length == offset)
        {
            /* h_free[i] is contiguous with and below freed slot */
            h_free[i].length += length;
            /* check whether revised slot is contiguous with slot above */
            if (i + 1 < h_head.free)
            {
                if (h_free[i].offset + h_free[i].length == h_free[i + 1].offset)
                {
                    /* Coalesce two slots */
                    h_free[i].length += h_free[i + 1].length;
                    /* Reshuffle rest of them downwards */
                    memmove(&h_free[i + 1], &h_free[i + 2], sizeof(Entry) * (h_head.free - i - 2));
                    h_head.free--;
                }
            }
            return;
        }
        else if (h_free[i].offset == slot_end)
        {
            /* h_free[i] is contiguous with and above freed slot */
            h_free[i].offset -= length;
            h_free[i].length += length;
            return;
        }
        else if (h_free[i].offset > offset)
            break;
    }
    /* New slot is not contiguous with any existing slot. */
    /* Insert new slot at appropriate point, which is indicated by i */
    if (i >= h_head.free)
    {
        h_free[h_head.free].offset = offset;
        h_free[h_head.free].length = length;
    }
    else
    {
        memmove(&h_free[i + 1], &h_free[i], sizeof(Entry) * (h_head.free - i));
        h_free[i].offset = offset;
        h_free[i].length = length;
    }
    h_head.free++;
}

/* Allocate history slot */
static Uint4 hist_alloc(Uint4 len)
{
    Uint4 i;
    Uint4 offset;

    for (i = 0; i < h_head.free; i++)
    {
        if (h_free[i].length >= len)
        {
            offset = h_free[i].offset;
            if (h_free[i].length > len)
            {
                h_free[i].offset += len;
                h_free[i].length -= len;
            }
            else
            {
                h_head.free--;
                memmove(&h_free[i], &h_free[i + 1], sizeof(Entry) * (h_head.free - i));
            }
            return(offset);
        }
    }
    offset = h_head.eof;
    h_head.eof += len;
    return(offset);
}

/*
** Normalise command numbers
** -- Negative numbers mean last number + given (negative) number.
** -- If result is less than minimum stored number, then minimum.
** -- Zero means last command.
** -- Reverse range if necessary.
** -- Treat out of range values as end of range.
*/
static void hist_normal(Sint4 *c1, Sint4 *c2)
{
    Sint4 n1 = *c1;
    Sint4 n2 = *c2;
    Sint4 mn = 1;
    Sint4 mx = h_head.count;

    if (h_head.count > h_head.size)
        mn = h_head.count - h_head.size;
    if (n1 <= -mx)
        n1 = -mx + 1;
    if (n1 <= 0)
        n1 += mx;
    if (n2 <= -mx)
        n2 = -mx + 1;
    if (n2 <= 0)
        n2 += mx;
    if (n1 > n2)
    {
        Sint4 t1 = n1;
        n1 = n2;
        n2 = t1;
    }
    *c1 = (n1 > mx) ? mx : (n1 < mn) ? mn : n1;
    *c2 = (n2 > mx) ? mx : (n2 < mn) ? mn : n2;
}

/* Range of history numbers available */
void hist_range(Sint4 *lower, Sint4 *upper)
{
    *lower = 1;
    *upper = 0;
    hist_normal(lower, upper);
}

/* Dump entire history file to given file */
void hist_dump(FILE *fp)
{
    Uint4 i;

    fprintf(fp, "** HISTORY DUMP **\n");
    fprintf(fp, "Size = %lu; Used = %lu; Free = %lu; ",
            (Ulong)h_head.size, (Ulong)h_head.used, (Ulong)h_head.free);
    fprintf(fp, "Count = %lu; Next = %lu; EOF = %lu\n",
            (Ulong)h_head.count, (Ulong)h_head.next, (Ulong)h_head.eof);
    fprintf(fp, "Used list:\n");
    for (i = 0; i < h_head.used; i++)
        fprintf(fp, "Used %2d: offset = %5lu; length = %5lu\n", (int)i,
                (Ulong)h_used[i].offset, (Ulong)h_used[i].length);
    fprintf(fp, "Free list:\n");
    for (i = 0; i < h_head.free; i++)
        fprintf(fp, "Free %2d: offset = %5ld; length = %5ld\n", (int)i,
                (Ulong)h_free[i].offset, (Ulong)h_free[i].length);
    fprintf(fp, "** END OF DUMP **\n");
    fflush(fp);
}

/* Set name of history file */
void hist_setfile(const char *name)
{
    h_name = name;
}

/* Name of current history file */
const char *hist_file(void)
{
    return(h_name);
}

/* Name of default history file */
static void hist_defaultfile(void)
{
    const char *name;
	if ((name = getenv(SQLCMD_HISTENV)) == NIL(const char *)
        || strstr(name, "..") != NULL
        || strchr(name, '/') != NULL
        || strchr(name, '\\') != NULL) {
        name = SQLCMD_HISTFILE;
	}
    hist_setfile(name);
}

/* Read history file header - architecture neutral format */
static int hist_read_header(FILE *fp, Header *h)
{
    int rc = 0;
    DiskHeader dh;

    if (fread(&dh, sizeof(DiskHeader), 1, fp) != 1)
        rc = -1;
    h->magic  = ld_uint4(dh.magic);
    h->size   = ld_uint4(dh.size);
    h->next   = ld_uint4(dh.next);
    h->count  = ld_uint4(dh.count);
    h->used   = ld_uint4(dh.used);
    h->free   = ld_uint4(dh.free);
    h->offset = ld_uint4(dh.offset);
    h->eof    = ld_uint4(dh.eof);
    return(rc);
}

/* Read history file entries - architecture neutral format */
static int hist_read_entries(FILE *fp, Entry *e, Uint4 n)
{
    int       rc = 0;
    DiskEntry de;
    Uint4     i;

    for (i = 0; i < n; i++)
    {
        if (fread(&de, sizeof(DiskEntry), 1, fp) != 1)
        {
            rc = -1;
            break;
        }
        e->offset = ld_uint4(de.offset);
        e->length = ld_uint4(de.length);
        e++;
    }
    return(rc);
}

/* Read history file trailer - architecture neutral format */
static int hist_read_trailer(FILE *fp)
{
    int rc = 0;
    char buffer[sizeof(SQLCMD_TRAILSTR)-1];

    if (fread(buffer, sizeof(buffer), 1, fp) != 1)
        rc = -1;
    else if (strncmp(buffer, SQLCMD_TRAILSTR, sizeof(buffer)) != 0)
        rc = -1;
    return(rc);
}

/* Write history file header - architecture neutral format */
static int hist_write_header(FILE *fp, Header *h)
{
    int rc = 0;
    DiskHeader dh;

    st_uint4(h->magic,  dh.magic);
    st_uint4(h->size,   dh.size);
    st_uint4(h->next,   dh.next);
    st_uint4(h->count,  dh.count);
    st_uint4(h->used,   dh.used);
    st_uint4(h->free,   dh.free);
    st_uint4(h->offset, dh.offset);
    st_uint4(h->eof,    dh.eof);

    if (fwrite(&dh, sizeof(DiskHeader), 1, fp) != 1)
        rc = -1;
    return(rc);
}

/* Write history file entries - architecture neutral format */
static int hist_write_entries(FILE *fp, Entry *e, Uint4 n)
{
    int rc = 0;
    DiskEntry de;
    Uint4 i;

    for (i = 0; i < n; i++)
    {
        st_uint4(e->offset, de.offset);
        st_uint4(e->length, de.length);
        e++;
        if (fwrite(&de, sizeof(DiskEntry), 1, fp) != 1)
        {
            rc = -1;
            break;
        }
    }
    return(rc);
}

/* Write history file trailer - architecture neutral format */
static int hist_write_trailer(FILE *fp)
{
    int rc = 0;

    if (fwrite(SQLCMD_TRAILSTR, sizeof(SQLCMD_TRAILSTR)-1, 1, fp) != 1)
        rc = -1;
    return(rc);
}

/* Flush control information for history file to disk */
static void hist_flush(FILE *fp)
{
    FUNCTION_NAME("hist_flush");

    if (fseek(fp, 0L, SEEK_SET))
        err_error(h_seekfail, __func__);
    else if (hist_write_header(fp, &h_head) != 0)
        err_error("write header failed in %s()\n", __func__);
    else if (hist_write_entries(fp, h_used, h_head.size) != 0)
        err_error("write used failed in %s()\n", __func__);
    else if (hist_write_entries(fp, h_free, h_head.size) != 0)
        err_error("write free failed in %s()\n", __func__);
    else if (hist_write_trailer(fp) != 0)
        err_error("write trailer failed in %s()\n", __func__);
    fflush(fp);
}

/* Read the history file control data */
/* The file is locked on entry; it will be closed if an error occurs (negative return); zero return for OK */
static int hist_read_control(void)
{
    FUNCTION_NAME("hist_read_control");
    assert(h_file != 0);
    if (fseek(h_file, 0L, SEEK_SET) != 0)
        err_syserr(h_seekfail, __func__);
    if (hist_read_header(h_file, &h_head) != 0)
    {
        /* Assume it is an empty (new) file */
        h_head.magic = SQLCMD_MAGIC2;
        h_head.size = SQLCMD_SIZE;
        h_head.next = 0;
        h_head.used = 0;
        h_head.free = 0;
        h_head.eof = 0;
        h_head.count = 0;
        h_head.offset = OFFSET(h_head.size);
        h_used = (Entry *)CALLOC(sizeof(Entry), h_head.size); /*=C++=*/
        h_free = (Entry *)CALLOC(sizeof(Entry), h_head.size); /*=C++=*/
        hist_flush(h_file);
    }
    else if (h_head.magic != SQLCMD_MAGIC2)
    {
        fclose(h_file);
        h_file = NIL(FILE *);
        return(h_head.magic == SQLCMD_MAGIC1 ? H_OLDMAGIC : H_BADMAGIC);
    }
    else
    {
        int failed = 0;
        size_t bytes = sizeof(Entry) * h_head.size;
        h_used = (Entry *)CALLOC(bytes, 1); /*=C++=*/
        h_free = (Entry *)CALLOC(bytes, 1); /*=C++=*/
        if (hist_read_entries(h_file, h_used, h_head.size) != 0)
            failed = 1;
        else if (hist_read_entries(h_file, h_free, h_head.size) != 0)
            failed = 1;
        else if (hist_read_trailer(h_file) != 0)
            failed = 1;
        if (failed == 1)
        {
            fclose(h_file);
            h_file = NIL(FILE *);
            FREE(h_used);
            FREE(h_free);
            h_used = NIL(Entry *);
            h_free = NIL(Entry *);
            return(H_BADFORMAT);
        }
    }
    return(0);
}

/* Open history file */
Sint4 hist_open(HistOpenMode mode)
{
    static int init_atexit = 0;
    Sint4 rc1 = 0;
    Sint4 rc2 = 0;
    FUNCTION_NAME("hist_open");

    if (init_atexit == 0)
    {
        atexit(hist_close);
        init_atexit = 1;
    }
    hist_close();
    if (h_name == NIL(const char *))
        hist_defaultfile();

    h_file = fopen(h_name, ((mode == H_READONLY) ? "rb" : "r+b"));
    if (h_file == NIL(FILE *) && mode != H_READONLY) {
        int fd = open(h_name, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
        if (fd >= 0)
        {
            h_file = fdopen(fd, "w+b");
            if (h_file == NIL(FILE *))
            {
                close(fd);
                return(H_CANTOPEN);
            }
        }
    }
    h_lock = (mode == H_READONLY) ? FLK_READLOCK : FLK_WRITELOCK;
    if ((rc1 = flk_waitlock(h_file, h_lock)) != 0)
    {
        err_sysrem(h_lock_error, __func__);
        return rc1;
    }

    /* KLUDGE: funny control logic - check */
    if ((rc1 = hist_read_control()) == 0)
    {
        if ((rc2 = flk_freelock(h_file)) < 0)
        {
            err_sysrem(h_unlk_error, __func__);
            rc1 = rc2;
        }
    }
    if (rc1 < 0)
        return rc1;
    return(h_head.count);
}

/* Close history file */
void hist_close(void)
{
    if (h_file != NIL(FILE *))
    {
        /* hist_flush() unnecessary - each hist_write() flushes changes */
        /* hist_flush(h_file); */
        fclose(h_file);
        h_file = NIL(FILE *);
        FREE(h_used);
        FREE(h_free);
        h_used = NIL(Entry *);
        h_free = NIL(Entry *);
        h_name = NIL(char *);
    }
}

/* Locate command in history file, giving offset and length */
static int hist_locate(Sint4 cmd, Entry **ep)
{
    Sint4  ucn;     /* User command number (UCN): 1..count */
    Sint4  mncn;    /* Minimum UCN available */
    Sint4  off;     /* Index in used list of UCN */

    if (cmd<= 0)
        ucn = cmd+ h_head.count;
    else
        ucn = cmd;
    mncn = MAX(1, h_head.count - h_head.size + 1);
    if (ucn < mncn || ucn > h_head.count)
        return(0);
    /* ucn is now a valid absolute user command number */
    off = h_head.next - 1;
    if (off < 0)
        off += h_head.size;
    off += (ucn - h_head.count);
    if (off < 0)
        off += h_head.size;
    assert(off >= 0 && off < (Sint4)h_head.size);
    *ep = &h_used[off];
    if (h_used[off].offset == 0 && h_used[off].length == 0)
        return(0);
    return(1);
}

static void hist_zapone(Sint4 cmd)
{
    Entry *ep;
    FUNCTION_NAME("hist_zapone");

    if (hist_locate(cmd, &ep))
    {
        char buffer[BUFSIZ];
        size_t tbytes = ep->length;
        size_t nbytes = MIN(sizeof(buffer), tbytes);

        /* Write newline as message - zap remainder with X's */
        buffer[0] = '\n';
        if (nbytes > 1)
            memset(&buffer[1], 'X', nbytes-1);
        if (fseek(h_file, ep->offset + h_head.offset, SEEK_SET))
            err_error(h_seekfail, __func__);
        while (tbytes > 0)
        {
            assert(tbytes >= nbytes);
            if (fwrite(buffer, sizeof(char), nbytes, h_file) != nbytes)
                err_error(h_writefail, __func__);
            tbytes -= nbytes;
            buffer[0] = 'X';
            nbytes = MIN(sizeof(buffer), tbytes);
        }

        /* Free residue and adjust claimed length */
        tbytes = ep->length - 1;
        ep->length = 1;
        hist_free(ep->offset+1, tbytes);
    }
}

/*
** Work out where to place a new record of length len.
** Position write marker to correct position in file.
** Calling routine undertakes not to write more than len bytes.
** Return: command number for command.
*/
static Uint4 hist_position(size_t len)
{
    Entry          *up;
    FUNCTION_NAME("hist_position");

    up = &h_used[h_head.next++];
    if (h_head.next >= h_head.size)
        h_head.next = 0;
    hist_free(up->offset, up->length);
    up->offset = hist_alloc(len);
    up->length = len;
    if (fseek(h_file, up->offset + h_head.offset, SEEK_SET))
        err_error(h_seekfail, __func__);
    if (h_head.used < h_head.size)
        h_head.used++;
    return(++h_head.count);
}

/* Write command to history file */
Uint4 hist_write(const char *cmd)
{
    Uint4  len;
    Uint4  cmdnum;
    char  *buff;
    FUNCTION_NAME("hist_write");

    if (h_file == NIL(FILE *))
        return(0);
    /* Do not use STRDUP() - we need to know the length */
    len = strlen(cmd) + 1;
    if (len == 1)
        return(0);
    buff = (char *)MALLOC(len + 1); /*=C++=*/
    strcpy(buff, cmd);
    buff[len - 1] = '\n';
    if (flk_waitlock(h_file, h_lock) != 0)
        return(0);      /* KLUDGE:? should be an error */
    cmdnum = hist_position(len);
    /* Write command string */
    if (fwrite(buff, sizeof(char), len, h_file) != len)
        err_error(h_writefail, __func__);
    hist_flush(h_file);
    if (flk_freelock(h_file) != 0)
        return(0);      /* KLUDGE:? should be an error */
    FREE(buff);
    return(cmdnum);
}

/* Internal version of hist_read() - history file is already locked */
static char *hist_read_internal(Sint4 cmdnum, const char *function)
{
    char  *cmd;
    Entry *ep;

    if (hist_locate(cmdnum, &ep) == 0)
        return(NIL(char(*)));
    else
    {
        if (fseek(h_file, ep->offset + h_head.offset, SEEK_SET))
        {
            err_sysrem(h_seekfail, function);
            return(NIL(char *));
        }
        cmd = (char *)MALLOC(ep->length); /*=C++=*/
        /* Read command string */
        if (fread(cmd, sizeof(char), ep->length, h_file) != ep->length)
        {
            err_sysrem(h_readfail, function);
            return(NIL(char *));
        }
        cmd[ep->length - 1] = '\0';
    }
    return(cmd);
}

/* Read command numbered... */
char *hist_read(Sint4 cmdnum)
{
    char  *cmd;
    FUNCTION_NAME("hist_read");

    if (h_file == NIL(FILE *))
        return(NIL(char *));    /* File not open */
    if (flk_waitlock(h_file, h_lock) != 0)
    {
        err_sysrem(h_lock_error, __func__);
        return(NIL(char *));
    }
    if (hist_read_control() != 0)
    {
        err_sysrem(h_read_error, __func__);
        return(NIL(char *));
    }
    cmd = hist_read_internal(cmdnum, __func__);
    if (flk_freelock(h_file) != 0)
    {
        err_sysrem(h_unlk_error, __func__);
        FREE(cmd);
        return(NIL(char *));
    }
    return(cmd);
}

/* Increase number of entries in history */
static void hist_expand(int newsize)
{
    FILE           *t_file;
    Uint4           osz;
    Uint4           off;
    Entry          *t_used;
    int             h1;
    int             u1;
    int             l1;

    osz = h_head.size;
    off = h_head.offset;
    h_head.size = newsize;
    h_head.offset = OFFSET(newsize);
    h_free = (Entry *)REALLOC(h_free, sizeof(Entry) * newsize); /*=C++=*/
    t_used = h_used;
    h_used = (Entry *)CALLOC(sizeof(Entry), newsize); /*=C++=*/
    if (h_head.used < osz)
    {
        memmove(h_used, t_used, sizeof(Entry) * osz);
    }
    else
    {
        h1 = 0;
        u1 = h_head.next;
        l1 = (osz - h_head.next) * sizeof(Entry);
        memmove(&h_used[h1], &t_used[u1], l1);
        h1 = osz - h_head.next;
        u1 = 0;
        l1 = h_head.next * sizeof(Entry);
        memmove(&h_used[h1], &t_used[u1], l1);
        h_head.next = osz;
    }
    FREE(t_used);

    /* Rewrite file to use new size information */
    t_file = tmpfile();
    hist_flush(t_file);         /* Write new header info to temp file */
    fseek(h_file, off, SEEK_SET);
    fcopy(h_file, t_file);      /* Copy old command info to temp file */
    fseek(h_file, 0L, SEEK_SET);
    fseek(t_file, 0L, SEEK_SET);
    fcopy(t_file, h_file);      /* Copy temp file over old file */
    fclose(t_file);
}

/* Decrease number of entries in history */
static void hist_shrink(int newsize)
{
    FILE           *o_file; /* Old file */
    Entry          *o_used; /* Old used list */
    Entry          *o_free; /* Old free list */
    Header          o_head; /* Old header */
    FILE           *t_file; /* New file */
    Entry          *t_used; /* New used list */
    Entry          *t_free; /* New free list */
    Header          t_head; /* New header */
    char           *cmd;
    int             h1;
    int             h2;
    int             i;

    o_file = h_file;
    o_used = h_used;
    o_free = h_free;
    o_head = h_head;

    t_file = tmpfile();
    t_head = h_head;
    t_used = (Entry *)CALLOC(sizeof(Entry), newsize); /*=C++=*/
    t_free = (Entry *)CALLOC(sizeof(Entry), newsize); /*=C++=*/
    t_head.size = newsize;
    t_head.offset = OFFSET(newsize);
    t_head.next = 0;
    t_head.count = 0;
    t_head.eof = 0;
    t_head.free = 0;
    t_head.used = 0;

    /* For each saved old message, read from old file, write to new */
    h1 = MAX(1, o_head.count - newsize + 1);
    h2 = o_head.count;
    for (i = h1; i <= h2; i++)
    {
        h_file = o_file;
        h_head = o_head;
        h_used = o_used;
        h_free = o_free;
        if ((cmd = hist_read(i)) != NIL(char *))
        {
            h_file = t_file;
            h_head = t_head;
            h_used = t_used;
            h_free = t_free;
            (void)hist_write(cmd);
            t_head = h_head;
            FREE(cmd);
        }
    }

    /* Write new header */
    if (h2 - h1 + 1 == newsize)
        t_head.next = 0;
    else
        t_head.next = h2;
    t_head.count = o_head.count;
    h_head = t_head;
    h_used = t_used;
    h_free = t_free;
    hist_flush(t_file);

    /* Copy temp file over truncated old file */
    fclose(o_file);
	int ofd = open(h_name, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
	if (ofd < 0 || (o_file = fdopen(ofd, "w+b")) == NIL(FILE *))
	{
		if (ofd >= 0)
			close(ofd);
		/* Should be able to re-open file. */
		/* Since we can't, shut down history */
		h_file = NIL(FILE *);
		FREE(t_used);
		FREE(t_free);
		FREE(o_used);
		FREE(o_free);
		fclose(t_file);
		return;
	}
    fseek(t_file, 0L, SEEK_SET);
    fcopy(t_file, o_file);

    /* Reset internal structures */
    h_head = t_head;
    h_used = t_used;
    h_free = t_free;
    h_file = o_file;            /* Yes; o_file! */

    /* Clean up */
    FREE(o_used);
    FREE(o_free);
    fclose(t_file);
}

/* Set number of commands saved in history file */
void hist_setsize(Uint4 newsize)
{
    FUNCTION_NAME("hist_setsize");

    if (h_file == NIL(FILE *))
        return;             /* File not open */
    else if (newsize <= 0)
        return;             /* Invalid size */
    else
    {
        if (flk_waitlock(h_file, h_lock) != 0)
            err_sysrem(h_lock_error, __func__);
        else
        {
            if (hist_read_control() != 0)
                err_sysrem(h_read_error, __func__);
            else if (newsize < h_head.size)
                hist_shrink(newsize);
            else if (newsize > h_head.size)
                hist_expand(newsize);
            if (flk_freelock(h_file) != 0)
                err_sysrem(h_unlk_error, __func__);
        }
    }
}

/* Read command from given file and log it in history file */
Uint4 hist_input(FILE *fp)
{
    size_t          nbytes;
    size_t          nread;
    char            cmd[BUFSIZ];
    struct stat     s;
    long            cmdnum;
    FUNCTION_NAME("hist_input");

    if (fstat(fileno(fp), &s) < 0 || s.st_size == 0)
        return(0);
    nbytes = s.st_size;

    if (flk_waitlock(h_file, h_lock) != 0)
    {
        err_sysrem(h_lock_error, __func__);
        return(0);
    }
    else if (hist_read_control() != 0)
    {
        err_sysrem(h_read_error, __func__);
        flk_freelock(h_file);
        return(0);
    }

    cmdnum = hist_position(nbytes);

    while (nbytes > 0)
    {
        /* Read command from input file */
        if ((nread = fread(cmd, sizeof(char), sizeof(cmd), fp)) == 0)
            break;
        if (nbytes < nread)
        {
            err_remark("%s(): read too many bytes (read %lu, want %lu) - surplus ignored\n",
                       __func__, (unsigned long)nread, (unsigned long)nbytes);
            nread = nbytes;
        }
        nbytes -= nread;
        /* Write command to history file */
        if (fwrite(cmd, sizeof(char), nread, h_file) != nread)
            err_sysrem("%s(): write failure on %s\n", __func__, h_name);
    }
    hist_flush(h_file);
    if (flk_freelock(h_file) != 0)
    {
        err_sysrem(h_unlk_error, __func__);
        return(0);
    }
    return(cmdnum);
}

/* Write list of commands (numbered c1..c2) to given file in text format */
void hist_output(FILE *fp, Sint4 cmd1, Sint4 cmd2, Uint4 flag)
{
    Sint4  i;
    char  *cmd;
    FUNCTION_NAME("hist_output");

    if (flk_waitlock(h_file, h_lock) != 0)
        err_sysrem(h_lock_error, __func__);
    else if (hist_read_control() != 0)
        err_sysrem(h_read_error, __func__);
    else
    {
        hist_normal(&cmd1, &cmd2);
        for (i = cmd1; i <= cmd2; i++)
        {
            cmd = hist_read_internal(i, __func__);
            if (cmd != NIL(char *))
            {
                if (flag == H_NUMBERS)
                    fprintf(fp, "-- %ld:\n", (long)i);
                fprintf(fp, "%s\n", cmd);
            }
            FREE(cmd);
        }
        if (flk_freelock(h_file) != 0)
            err_sysrem(h_unlk_error, __func__);
    }
}

/* Erase list of commands (numbered cmd1..cmd2) from history file */
void hist_erase(Sint4 cmd1, Sint4 cmd2)
{
    Sint4  i;
    FUNCTION_NAME("hist_erase");

    if (flk_waitlock(h_file, h_lock) != 0)
        err_sysrem(h_lock_error, __func__);
    else if (hist_read_control() != 0)
        err_sysrem(h_read_error, __func__);
    else
    {
        hist_normal(&cmd1, &cmd2);
        for (i = cmd1; i <= cmd2; i++)
            hist_zapone(i);
        if (flk_freelock(h_file) != 0)
            err_sysrem(h_unlk_error, __func__);
        hist_flush(h_file);
        if (flk_freelock(h_file) != 0)
            err_sysrem(h_unlk_error, __func__);
    }
}

/* Number of commands saved in history file */
Uint4 hist_getsize(void)
{
    Uint4 rc = 0;
    FUNCTION_NAME("hist_getsize");

    if (flk_waitlock(h_file, h_lock) != 0)
        err_sysrem(h_lock_error, __func__);
    else if (hist_read_control() != 0)
        err_sysrem(h_read_error, __func__);
    else if (flk_freelock(h_file) != 0)
        err_sysrem(h_unlk_error, __func__);
    else
        rc = h_head.size;
    return(rc);
}

/* Get current command number */
Uint4 hist_getcmdnum(void)
{
    Uint4 rc = 0;
    FUNCTION_NAME("hist_getcmdnum");

    if (flk_waitlock(h_file, h_lock) != 0)
        err_sysrem(h_lock_error, __func__);
    else if (hist_read_control() != 0)
        err_sysrem(h_read_error, __func__);
    else if (flk_freelock(h_file) != 0)
        err_sysrem(h_unlk_error, __func__);
    else
        rc = h_head.count;
    return(rc);
}
