/*
@(#)File:           $RCSfile: dumpblob.ec,v $
@(#)Version:        $Revision: 2013.1 $
@(#)Last changed:   $Date: 2013/09/08 22:11:16 $
@(#)Purpose:        Dump/interpret contents of BLOB structure
@(#)Author:         J Leffler
@(#)Copyright:      (C) JLSS 1992,1995-2000,2003,2008,2013
@(#)Product:        SQLCMD Version 90.02 (2016-07-28)
*/

/*TABSTOP=4*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

/* This is an ESQL/C file to simplify the compiling.  */
#include "dumpesql.h"
#include "esqlutil.h"
#include "esqltype.h"
#include "dumpconfig.h"

#define loc_mode    lc_union.lc_file.lc_mode

static const char *loc_types[] =
{
    "<<undefined type>>",
    "LOCMEMORY",
    "LOCFNAME",
    "LOCFILE",
    "LOCUSER",
};

#ifndef lint
/* Prevent over-aggressive optimizers from eliminating ID string */
const char jlss_id_dumpblob_ec[] = "@(#)$Id: dumpblob.ec,v 2013.1 2013/09/08 22:11:16 jleffler Exp $";
#endif /* lint */

static const char *loc_type(size_t type)
{
    if (type >= DIM(loc_types))
        return loc_types[0];
    return loc_types[type];
}

/* Print details about blob structure to standard error */
void dump_blob(FILE *fp, const char *tag, const Blob *blob)
{
    if (tag == (char *)0)
        tag = "";
    putc('\n', fp);
    dump_print(fp, "BYTE/TEXT: %s -- address = 0x%08" PRIXPTR "\n", tag, (uintptr_t)blob);
    dump_print(fp, "Locator type = %d (%s)\n", blob->loc_loctype,
            loc_type(blob->loc_loctype));
    dump_print(fp, "blob size = %" PRId_ixInt4 ", indicator = %" PRId_ixInt4 "\n",
            blob->loc_size, blob->loc_indicator);
    dump_print(fp, "blob type = %" PRId_ixInt4 ", blob status = %" PRId_ixMint "n",
            blob->loc_type, blob->loc_status);
    dump_print(fp, "transfer count = %" PRId_ixInt4 "\n", blob->loc_xfercount);
    if (blob->loc_loctype == LOCMEMORY)
    {
        dump_print(fp, "Buffer       = 0x%08" PRIXPTR "\n", (uintptr_t)blob->loc_buffer);
        dump_print(fp, "Buffer size  = %" PRId_ixInt4 "\n", blob->loc_bufsize);
        dump_print(fp, "Memory flags = 0x%02X\n", blob->loc_mflags);
    }
    else if (blob->loc_loctype == LOCFNAME)
    {
        dump_print(fp, "Filename = %s\n", blob->loc_fname);
        dump_print(fp, "Mode     = %04o\n", blob->loc_mode);
        dump_print(fp, "Flags    = 0x%02X\n", blob->loc_oflags);
    }
    else if (blob->loc_loctype == LOCFILE)
    {
        dump_print(fp, "File descriptor = %d\n", blob->loc_fd);
        dump_print(fp, "Flags           = 0x%02X\n", blob->loc_oflags);
    }
    else if (blob->loc_loctype == LOCUSER)
    {
        dump_print(fp, "Open  function = 0x%08" PRIXPTR "\n", (uintptr_t)blob->loc_open);
        dump_print(fp, "Read  function = 0x%08" PRIXPTR "\n", (uintptr_t)blob->loc_read);
        dump_print(fp, "Write function = 0x%08" PRIXPTR "\n", (uintptr_t)blob->loc_write);
        dump_print(fp, "Close function = 0x%08" PRIXPTR "\n", (uintptr_t)blob->loc_close);
    }
    else
    {
        dump_print(fp, "locator type = %d = 0x%08X is invalid\n",
                blob->loc_loctype, blob->loc_loctype);
    }
    putc('\n', fp);
    fflush(fp);
}
