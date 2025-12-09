/*
@(#)File:           $RCSfile: dumpsqlda.ec,v $
@(#)Version:        $Revision: 2008.5 $
@(#)Last changed:   $Date: 2008/04/07 05:53:17 $
@(#)Purpose:        Print contents of SQLDA structure
@(#)Author:         J Leffler
@(#)Copyright:      (C) JLSS 1987,1991,1995-2004,2007-08
@(#)Product:        SQLCMD Version 90.02 (2016-07-28)
*/

/*TABSTOP=4*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include "dumpesql.h"
#include "esqlutil.h"
#include "esqltype.h"
#include "dumpconfig.h"

#ifdef DEBUG_IMAGEPRINT
#include "imageprt.h"
#endif /* DEBUG_IMAGEPRINT */

/* LVCHARTYPE?  LVCHARPTRTYPE? */
#define is_c_chartype(x)    ((x) == CCHARTYPE || (x) == CVCHARTYPE || (x) == CFIXCHARTYPE || (x) == CSTRINGTYPE)
#define is_sql_chartype(x)  ((x) == SQLCHAR || (x) == SQLVCHAR || (x) == SQLNCHAR || (x) == SQLNVCHAR)

struct Typename
{
    const char  *name;
    int          code;
};

typedef struct Typename Typename;

#ifdef ESQLC_IUSTYPES
static const char header[] = "%-5s %-14s%-20s %-6s %-14s %-8s %-14s%-14s%-7s %-6s %-.20s\n";
static const char format[] = "%-5d 0x%08" PRIXPTR "    %-20s %-6d %-14s %-8" PRId_ixInt4 " 0x%08" PRIXPTR "    0x%08" PRIXPTR "    %-7s %-6" PRId_ixInt4 " %-s\n";
#else
static const char header[] = "%-5s %-14s%-20s %-6s %-14s %-8s %-14s%-14s\n";
static const char format[] = "%-5d 0x%08" PRIXPTR "    %-20s %-6d %-14s %-8" PRId_ixInt4 " 0x%08" PRIXPTR "\n";
#endif /* ESQLC_IUSTYPES */

static const char no_name[] = "<NO NAME>";

/**INDENT-OFF**/
static const Typename sql_typenames[] =
{
    { "CBOOLTYPE",      CBOOLTYPE       },
    { "CCHARTYPE",      CCHARTYPE       },
    { "CCOLLTYPE",      CCOLLTYPE       },
    { "CDATETYPE",      CDATETYPE       },
    { "CDECIMALTYPE",   CDECIMALTYPE    },
    { "CDOUBLETYPE",    CDOUBLETYPE     },
    { "CDTIMETYPE",     CDTIMETYPE      },
    { "CFILETYPE",      CFILETYPE       },
    { "CFIXBINTYPE",    CFIXBINTYPE     },
    { "CFIXCHARTYPE",   CFIXCHARTYPE    },
    { "CFLOATTYPE",     CFLOATTYPE      },
    { "CINT8TYPE",      CINT8TYPE       },
    { "CINTTYPE",       CINTTYPE        },
    { "CINVTYPE",       CINVTYPE        },
    { "CLOCATORTYPE",   CLOCATORTYPE    },
    { "CLONGTYPE",      CLONGTYPE       },
    { "CLVCHARPTRTYPE", CLVCHARPTRTYPE  },
    { "CLVCHARTYPE",    CLVCHARTYPE     },
    { "CMONEYTYPE",     CMONEYTYPE      },
    { "CROWTYPE",       CROWTYPE        },
    { "CSHORTTYPE",     CSHORTTYPE      },
    { "CSTRINGTYPE",    CSTRINGTYPE     },
    { "CVARBINTYPE",    CVARBINTYPE     },
    { "CVCHARTYPE",     CVCHARTYPE      },
    { "SQLBOOL",        SQLBOOL         },
    { "SQLBYTES",       SQLBYTES        },
    { "SQLCHAR",        SQLCHAR         },
    { "SQLCOLLECTION",  SQLCOLLECTION   },
    { "SQLDATE",        SQLDATE         },
    { "SQLDECIMAL",     SQLDECIMAL      },
    { "SQLDTIME",       SQLDTIME        },
    { "SQLFLOAT",       SQLFLOAT        },
    { "SQLIMPEXP",      SQLIMPEXP       },
    { "SQLIMPEXPBIN",   SQLIMPEXPBIN    },
    { "SQLINT",         SQLINT          },
    { "SQLINT8",        SQLINT8         },
    { "SQLINTERVAL",    SQLINTERVAL     },
    { "SQLLIST",        SQLLIST         },
    { "SQLLVARCHAR",    SQLLVARCHAR     },
    { "SQLMONEY",       SQLMONEY        },
    { "SQLMULTISET",    SQLMULTISET     },
    { "SQLNCHAR",       SQLNCHAR        },
    { "SQLNULL",        SQLNULL         },
    { "SQLNVCHAR",      SQLNVCHAR       },
    { "SQLREFSER8",     SQLREFSER8      },
    { "SQLROW",         SQLROW          },
    { "SQLROWREF",      SQLROWREF       },
    { "SQLSENDRECV",    SQLSENDRECV     },
    { "SQLSERIAL",      SQLSERIAL       },
    { "SQLSERIAL8",     SQLSERIAL8      },
    { "SQLSET",         SQLSET          },
    { "SQLSMFLOAT",     SQLSMFLOAT      },
    { "SQLSMINT",       SQLSMINT        },
    { "SQLTEXT",        SQLTEXT         },
    { "SQLUDTFIXED",    SQLUDTFIXED     },
    { "SQLUDTVAR",      SQLUDTVAR       },
    { "SQLVCHAR",       SQLVCHAR        },
};
/**INDENT-ON**/

#ifndef lint
/* Prevent over-aggressive optimizers from eliminating ID string */
const char jlss_id_dumpsqlda_ec[] = "@(#)$Id: dumpsqlda.ec,v 2008.5 2008/04/07 05:53:17 jleffler Exp $";
#endif /* lint */

static const char *sql_typename(int type)
{
    size_t            i;

    for (i = 0; i < DIM(sql_typenames); i++)
    {
        if (sql_typenames[i].code == type)
            return(sql_typenames[i].name);
    }
    ESQLC_VERSION_CHECKER();
    return(0);
}

void dump_sqlva(FILE *fp, int item, const Sqlva *sqlvar)
{
    const char     *jl_sqltype;
    const char     *name;
#ifdef ESQLC_IUSTYPES
    const char     *sv_sqltype;
    const char     *nullity;
#endif /* ESQLC_IUSTYPES */
    char buffer[SQLTYPENAME_BUFSIZ];

    if (ISSQLTYPE(sqlvar->sqltype))
        jl_sqltype = sqltypename(sqlvar->sqltype, sqlvar->sqllen, buffer, sizeof(buffer));
    else
        jl_sqltype = sql_typename(sqlvar->sqltype);

    name = sqlvar->sqlname;
    if (sqlvar->sqlname == (char *)0)
        name = no_name;

#ifdef ESQLC_IUSTYPES
    sv_sqltype = sqlvar->sqltypename;
    if (sqlvar->sqltypename == 0)
        sv_sqltype = no_name;

    /* The sqlind field is always available in sqlda structure, but is not printed in 5.x */
    if (sqlvar->sqlind == 0)
        nullity = "unknown";
    else if (*sqlvar->sqlind != 0)
        nullity = "null";
    else
        nullity = "notnull";

    dump_print(fp, format, item, (uintptr_t)name, name, sqlvar->sqltype, jl_sqltype,
            sqlvar->sqllen, (uintptr_t)sqlvar->sqldata,
            (uintptr_t)sqlvar->sqlind, nullity, sqlvar->sqlxid, sv_sqltype);
#else
    dump_print(fp, format, item, (uintptr_t)name, name, sqlvar->sqltype, jl_sqltype,
            sqlvar->sqllen, (uintptr_t)sqlvar->sqldata);
#endif /* ESQLC_IUSTYPES */

#ifdef DEBUG_IMAGEPRINT
    if (sqlvar->sqldata != 0 && (is_sql_chartype(sqlvar->sqltype) || is_c_chartype(sqlvar->sqltype)))
        image_print(fp, 0L, sqlvar->sqldata, sqlvar->sqllen);
#endif /* DEBUG_IMAGEPRINT */
}

void            dump_sqlda(FILE *fp, const char *tag, const Sqlda *desc)
{
    int             i;

    if (tag == (char *)0)
        tag = "";
    dump_print(fp, "*** DUMP SQLDA *** %s\n", tag);
    dump_print(fp, "Address: 0x%08" PRIXPTR "\n", (uintptr_t)desc);
    if (desc != 0)
    {
        dump_print(fp, "Items:   %" PRId_ixInt2 "\n", desc->sqld);
#ifdef ESQLC_IUSTYPES
        dump_print(fp, header, "Item", "Name Address", "Column Name", "Type",
                "Type Name", "Length", "Data Address", "Indicator", "Nullity", "XID", "Extended Type Name");
#else
        dump_print(fp, header, "Item", "Name Address", "Column Name", "Type",
                "Type Name", "Length", "Data Address", "Indicator");
#endif /* ESQLC_IUSTYPES */

        for (i = 0; i < desc->sqld; i++)
        {
            dump_sqlva(fp, i, &desc->sqlvar[i]);
        }
    }
    dump_print(fp, "******************\n");
    fflush(fp);
}

$ifdef TEST;

#include <stdlib.h>

static void sql_error(void)
{
    sql_printerror(stderr);
    exit(1);
}

int main(int argc, char **argv)
{
    EXEC SQL BEGIN DECLARE SECTION;
    char *dbname = "stores7";
    char *stmt = "SELECT * FROM Orders";
    EXEC SQL END DECLARE SECTION;
    Sqlda *ptr;

    if (argc > 1)
        dbname = argv[1];
    if (argc > 2)
        stmt = argv[2];

    EXEC SQL WHENEVER ERROR CALL sql_error;
    EXEC SQL DATABASE :dbname;
    EXEC SQL PREPARE p_stmt FROM :stmt;
    EXEC SQL DESCRIBE p_stmt INTO ptr;
    printf("Statement: %s\n", stmt);
    dump_sqlda(stdout, "Description of statment", ptr);
    return(0);
}

$endif; /* TEST */

