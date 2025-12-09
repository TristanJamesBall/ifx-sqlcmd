#include <sqlhdr.h>
#include <sqliapi.h>
static const char _Cn2[] = "c_statement";
static const char _Cn1[] = "s_statement";
#line 1 "sqlread-sqlstmt.ec"
/*
@(#)File:           $RCSfile: sqlstmt.ec,v $
@(#)Version:        $Revision: 2015.1 $
@(#)Last changed:   $Date: 2015/06/24 07:32:10 $
@(#)Purpose:        Process SQL statements
@(#)Author:         J Leffler
@(#)Copyright:      (C) JLSS 1992-93,1995-97,1999-2008,2015
@(#)Product:        SQLCMD Version 90.02 (2016-07-28)
*/

/* -- Include Files -- */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "connect.h"
#include "context.h"
#include "debug.h"
#include "emalloc.h"
#include "esqlc.h"
#include "describe.h"
#include "internal.h"
#include "output.h"
#include "sqlcmd.h"
#include "sqlerr.h"
#include "jlsstools.h"
#include "esnprintf.h"
#include "jlss.h"

#include "stderr.h"

/* -- Type Definitions -- */

typedef struct CodeName
{
    int                code;
    const char * const name;
} CodeName;

/* -- Constant Definitions -- */

#define MAXDBS    100
#define FASIZ     (MAXDBS * 128)

/* -- Declarations -- */

static int stmts_sorted = 0;
static const char intuse[] = "for internal use only";
static const char reserved[] = "reserved for future use";

/* B165572: bug in sqlstype.h */
#ifndef SQ_CREATEROLE
#define SQ_CREATEROLE 83
#endif  /* SQ_CREATEROLE */
#ifndef SQ_DROPROLE
#define SQ_DROPROLE 84
#endif  /* SQ_DROPROLE */
#ifndef SQ_SETROLE
#define SQ_SETROLE 85
#endif  /* SQ_SETROLE */
#ifndef SQ_PASSWD
#define SQ_PASSWD 86
#endif  /* SQ_PASSWD */

/* Maintain in order - you will get a runtime warning if they are out of order */
static CodeName stmt_names[] =
{
    { SQ_DATABASE,      "DATABASE"                          },
    { SQ_SELECT,        "SELECT"                            },
    { SQ_SELINTO,       "SELECT INTO TEMP"                  },
    { SQ_UPDATE,        "UPDATE...WHERE"                    },
    { SQ_DELETE,        "DELETE...WHERE"                    },
    { SQ_INSERT,        "INSERT"                            },
    { SQ_UPDCURR,       "UPDATE WHERE CURRENT OF"           },
    { SQ_DELCURR,       "DELETE WHERE CURRENT OF"           },
    { SQ_LDINSERT,      intuse                              },
    { SQ_LOCK,          "LOCK TABLE"                        },
    { SQ_UNLOCK,        "UNLOCK TABLE"                      },
    { SQ_CREADB,        "CREATE DATABASE"                   },
    { SQ_DROPDB,        "DROP DATABASE"                     },
    { SQ_CRETAB,        "CREATE TABLE"                      },
    { SQ_DRPTAB,        "DROP TABLE"                        },
    { SQ_CREIDX,        "CREATE INDEX"                      },
    { SQ_DRPIDX,        "DROP INDEX"                        },
    { SQ_GRANT,         "GRANT"                             },
    { SQ_REVOKE,        "REVOKE"                            },
    { SQ_RENTAB,        "RENAME TABLE"                      },
    { SQ_RENCOL,        "RENAME COLUMN"                     },
    { SQ_CREAUD,        "CREATE AUDIT"                      },
    { SQ_STRAUD,        intuse                              },
    { SQ_STPAUD,        intuse                              },
    { SQ_DRPAUD,        "DROP AUDIT"                        },
    { SQ_RECTAB,        "RECOVER TABLE"                     },
    { SQ_CHKTAB,        intuse                              },
    { SQ_REPTAB,        intuse                              },
    { SQ_ALTER,         "ALTER TABLE"                       },
    { SQ_STATS,         "UPDATE STATISTICS"                 },
    { SQ_CLSDB,         "CLOSE DATABASE"                    },
    { SQ_DELALL,        "DELETE (no WHERE)"                 },
    { SQ_UPDALL,        "UPDATE (no WHERE)"                 },
    { SQ_BEGWORK,       "BEGIN WORK"                        },
    { SQ_COMMIT,        "COMMIT WORK"                       },
    { SQ_ROLLBACK,      "ROLLBACK WORK"                     },
    { SQ_SAVEPOINT,     intuse                              },
    { SQ_STARTDB,       "START DATABASE"                    },
    { SQ_RFORWARD,      "ROLLFORWARD DATABASE"              },
    { SQ_CREVIEW,       "CREATE VIEW"                       },
    { SQ_DROPVIEW,      "DROP VIEW"                         },
    { SQ_DEBUG,         intuse                              },
    { SQ_CREASYN,       "CREATE SYNONYM"                    },
    { SQ_DROPSYN,       "DROP SYNONYM"                      },
    { SQ_CTEMP,         "CREATE TEMP TABLE"                 },
    { SQ_WAITFOR,       "SET LOCK MODE"                     },
    { SQ_ALTIDX,        "ALTER INDEX"                       },
    { SQ_ISOLATE,       "SET ISOLATION"                     },
    { SQ_SETLOG,        "SET LOG"                           },
    { SQ_EXPLAIN,       "SET EXPLAIN"                       },
    { SQ_SCHEMA,        "CREATE SCHEMA"                     },
    { SQ_OPTIM,         "SET OPTIMIZATION"                  },
    { SQ_CREPROC,       "CREATE PROCEDURE"                  },
    { SQ_DRPPROC,       "DROP PROCEDURE"                    },
    { SQ_CONSTRMODE,    "SET CONSTRAINTS"                   },
    { SQ_EXECPROC,      "EXECUTE PROCEDURE"                 },
    { SQ_DBGFILE,       "SET DEBUG FILE TO"                 },
    { SQ_CREOPCL,       "CREATE OPTICAL CLUSTER"            },
    { SQ_ALTOPCL,       "ALTER OPTICAL CLUSTER"             },
    { SQ_DRPOPCL,       "DROP OPTICAL CLUSTER"              },
    { SQ_OPRESERVE,     "RESERVE (optical)"                 },
    { SQ_OPRELEASE,     "RELEASE (optical)"                 },
    { SQ_OPTIMEOUT,     "SET OPTICAL TIMEOUT"               },
    { SQ_PROCSTATS,     "UPDATE STATISTICS FOR PROCEDURE"   },
    { SQ_CRETRIG,       "CREATE TRIGGER"                    },
    { SQ_DRPTRIG,       "DROP TRIGGER"                      },
    /* The following codes are not found in ESQL/C 5.20 */
#ifdef SQ_UNKNOWN
    { SQ_UNKNOWN,       intuse                              },
#endif /* SQ_UNKNOWN */
#ifdef SQ_SETDATASKIP
    { SQ_SETDATASKIP,   "SET DATASKIP"                      },
#endif /* SQ_SETDATASKIP */
#ifdef SQ_PDQPRIORITY
    { SQ_PDQPRIORITY,   "SET PDQPRIORITY"                   },
#endif /* SQ_PDQPRIORITY */
#ifdef SQ_ALTFRAG
    { SQ_ALTFRAG,       "ALTER FRAGMENT"                    },
#endif /* SQ_ALTFRAG */
#ifdef SQ_SETOBJMODE
    { SQ_SETOBJMODE,    "SET <object> MODE"                 },
#endif /* SQ_SETOBJMODE */
#ifdef SQ_START
    { SQ_START,         "START VIOLATIONS TABLE"            },
#endif /* SQ_START */
#ifdef SQ_STOP
    { SQ_STOP,          "STOP VIOLATIONS TABLE"             },
#endif /* SQ_STOP */
#ifdef SQ_SETMAC
    { SQ_SETMAC,        "SET SESSION LEVEL"                 },
#endif /* SQ_SETMAC */
#ifdef SQ_SETDAC
    { SQ_SETDAC,        "SET SESSION AUTHORIZATION"         },
#endif /* SQ_SETDAC */
#ifdef SQ_SETTBLHI
    { SQ_SETTBLHI,      "SET TABLE HIGH"                    },
#endif /* SQ_SETTBLHI */
#ifdef SQ_SETLVEXT
    { SQ_SETLVEXT,      "SET EXTENT SIZE"                   },
#endif /* SQ_SETLVEXT */
#ifdef SQ_CREATEROLE
    { SQ_CREATEROLE,    "CREATE ROLE"                       },
#endif /* SQ_CREATEROLE */
#ifdef SQ_DROPROLE
    { SQ_DROPROLE,      "DROP ROLE"                         },
#endif /* SQ_DROPROLE */
#ifdef SQ_SETROLE
    { SQ_SETROLE,       "SET ROLE"                          },
#endif /* SQ_SETROLE */
#ifdef SQ_PASSWD
    { SQ_PASSWD,        "SET DBPASSWORD"                    },
#endif /* SQ_PASSWD */
#ifdef SQ_RENDB
    { SQ_RENDB,         "RENAME DATABASE"                   },
#endif /* SQ_RENDB */
#ifdef SQ_CREADOM
    { SQ_CREADOM,       "CREATE DOMAIN"                     },
#endif /* SQ_CREADOM */
#ifdef SQ_DROPDOM
    { SQ_DROPDOM,       "DROP DOMAIN"                       },
#endif /* SQ_DROPDOM */
#ifdef SQ_CREANRT
    { SQ_CREANRT,       "CREATE NAMED ROW TYPE"             },
#endif /* SQ_CREANRT */
#ifdef SQ_DROPNRT
    { SQ_DROPNRT,       "DROP NAMED ROW TYPE"               },
#endif /* SQ_DROPNRT */
#ifdef SQ_CREADT
    { SQ_CREADT,        "CREATE DISTINCT TYPE"              },
#endif /* SQ_CREADT */
#ifdef SQ_CREACT
    { SQ_CREACT,        "CREATE CAST"                       },
#endif /* SQ_CREACT */
#ifdef SQ_DROPCT
    { SQ_DROPCT,        "DROP CAST"                         },
#endif /* SQ_DROPCT */
#ifdef SQ_CREABT
    { SQ_CREABT,        "CREATE OPAQUE TYPE"                },
#endif /* SQ_CREABT */
#ifdef SQ_DROPTYPE
    { SQ_DROPTYPE,      "DROP TYPE"                         },
#endif /* SQ_DROPTYPE */
#ifdef SQ_ALTERROUTINE
    { SQ_ALTERROUTINE,  "ALTER ROUTINE"                     },
#endif /* SQ_ALTERROUTINE */
#ifdef SQ_CREATEAM
    { SQ_CREATEAM,      "CREATE ACCESS METHOD"              },
#endif /* SQ_CREATEAM */
#ifdef SQ_DROPAM
    { SQ_DROPAM,        "DROP ACCESS METHOD"                },
#endif /* SQ_DROPAM */
#ifdef SQ_ALTERAM
    { SQ_ALTERAM,       "ALTER ACCESS METHOD"               },
#endif /* SQ_ALTERAM */
#ifdef SQ_CREATEOPC
    { SQ_CREATEOPC,     "CREATE OPCLASS"                    },
#endif /* SQ_CREATEOPC */
#ifdef SQ_DROPOPC
    { SQ_DROPOPC,       "DROP OPCLASS"                      },
#endif /* SQ_DROPOPC */
#ifdef SQ_CREACST
    { SQ_CREACST,       "CREATE CONSTRUCTOR"                },
#endif /* SQ_CREACST */
#ifdef SQ_SETRES
    { SQ_SETRES,        "SET TABLE RESIDENCY"               },
#endif /* SQ_SETRES */
#ifdef SQ_CREAGG
    { SQ_CREAGG,        "CREATE AGGREGATE"                  },
#endif /* SQ_CREAGG */
#ifdef SQ_DRPAGG
    { SQ_DRPAGG,        "DROP AGGREGATE"                    },
#endif /* SQ_DRPAGG */
#ifdef SQ_PLOADFILE
    { SQ_PLOADFILE,     intuse                              },
#endif /* SQ_PLOADFILE */
#ifdef SQ_CHKIDX
    { SQ_CHKIDX,        intuse                              },
#endif /* SQ_CHKIDX */
#ifdef SQ_SCHEDULE
    { SQ_SCHEDULE,      "SET SCHEDULE"                      },
#endif /* SQ_SCHEDULE */
#ifdef SQ_SETENV
    { SQ_SETENV,        "SET ENVIRONMENT"                   },
#endif /* SQ_SETENV */
#ifdef SQ_CREADUP
    { SQ_CREADUP,       "CREATE DUPLICATE"                  },
#endif /* SQ_CREADUP */
#ifdef SQ_DROPDUP
    { SQ_DROPDUP,       "DROP DUPLICATE"                    },
#endif /* SQ_DROPDUP */
#ifdef SQ_XPS_RES4
    { SQ_XPS_RES4,      reserved                            },
#endif /* SQ_XPS_RES4 */
#ifdef SQ_XPS_RES5
    { SQ_XPS_RES5,      reserved                            },
#endif /* SQ_XPS_RES5 */
#ifdef SQ_STMT_CACHE
    { SQ_STMT_CACHE,    "SET STMT_CACHE"                    },
#endif /* SQ_STMT_CACHE */
#ifdef SQ_RENIDX
    { SQ_RENIDX,        "RENAME INDEX"                      },
#endif /* SQ_RENIDX */
#ifdef SQ_CREAGUID
    { SQ_CREAGUID,      "CREATE SERVERGUID"                 },
#endif /* SQ_CREAGUID */
#ifdef SQ_DROPGUID
    { SQ_DROPGUID,      "DROP SERVERGUID"                   },
#endif /* SQ_DROPGUID */
#ifdef SQ_ALTRGUID
    { SQ_ALTRGUID,      "ALTER SERVERGUID"                  },
#endif /* SQ_ALTRGUID */
#ifdef SQ_ALTERBT
    { SQ_ALTERBT,       "ALTER TYPE"                        },
#endif /* SQ_ALTERBT */
#ifdef SQ_ALTERCST
    { SQ_ALTERCST,      "ALTER CONSTRUCTOR"                 },
#endif /* SQ_ALTERCST */
#ifdef SQ_TRUNCATE
    { SQ_TRUNCATE,      "TRUNCATE TABLE"                    },
#endif /* SQ_TRUNCATE */
#ifdef SQ_IMPLICITTX
    { SQ_IMPLICITTX,    "IMPLICIT TRANSACTION"              },
#endif /* SQ_IMPLICITTX */
#ifdef SQ_CRESEQ
    { SQ_CRESEQ,        "CREATE SEQUENCE"                   },
#endif /* SQ_CRESEQ */
#ifdef SQ_DRPSEQ
    { SQ_DRPSEQ,        "DROP SEQUENCE"                     },
#endif /* SQ_DRPSEQ */
#ifdef SQ_ALTERSEQ
    { SQ_ALTERSEQ,      "ALTER SEQUENCE"                    },
#endif /* SQ_ALTERSEQ */
#ifdef SQ_RENSEQ
    { SQ_RENSEQ,        "RENAME SEQUENCE"                   },
#endif /* SQ_RENSEQ */
};

#ifndef lint
/* Prevent over-aggressive optimizers from eliminating ID string */
extern const char jlss_id_sqlstmt_ec[];
const char jlss_id_sqlstmt_ec[] = "@(#)$Id: sqlstmt.ec,v 2015.1 2015/06/24 07:32:10 jleffler Exp $";
#endif /* lint */

static int cmp_codenames(const void *vp1, const void *vp2)
{
    CodeName *p1 = (CodeName *)vp1;
    CodeName *p2 = (CodeName *)vp2;
    return(p1->code - p2->code);
}

static void check_stmt_codes(void)
{
    size_t i;

    for (i = 0; i < DIM(stmt_names) - 1; i++)
    {
        if (stmt_names[i].code >= stmt_names[i+1].code)
        {
            /* Table up to i is ordered; thereafter, dubious.  Fix it! */
            cmd_warning(E_INTERNAL, "table of statement names not in sorted order");
            qsort(&stmt_names[i], DIM(stmt_names) - i, sizeof(CodeName), cmp_codenames);
            break;
        }
    }
    stmts_sorted = 1;
}

static const char *sql_stmttype(int code)
{
    CodeName *stmt = 0;
    const char *rv = "unknown SQL statement";

    if (stmts_sorted == 0)
        check_stmt_codes();
    if (code > 0)
    {
        /*
        ** JL 2005-12-12: Sun C compiler warns about non-constant op:
        ** NAME if written as: CodeName s = { code, 0 };
        */
        CodeName s = { 0, 0 };
        s.code = code;
        stmt = (CodeName *)bsearch(&s, stmt_names, DIM(stmt_names), sizeof(CodeName), cmp_codenames);   /*=C++=*/
    }
    if (stmt != 0)
        rv = stmt->name;
    return rv;
}

/* print_serials() - print assigned serial values */
/* only one of SERIAL8 or BIGSERIAL is effective  */
static void print_serials(void)
{
    FILE *fp = ctxt_error();
    const char *pad = "";
#ifdef ESQLC_IUSTYPES
    int         did_serial8 = 0;
    ifx_int8_t i8;
    ifx_int8_t zero;
    int        rc;
#endif /* ESQLC_IUSTYPES */

    if (sqlca.sqlerrd[1] != 0)
    {
        fprintf(fp, "%s%s = %" PRId_ixInt4, pad, "SERIAL", sqlca.sqlerrd[1]);
        pad = ", ";
    }

#ifdef ESQLC_IUSTYPES
    ifx_int8cvint(0, &zero);
    ifx_getserial8(&i8);
    rc = ifx_int8cmp(&i8, &zero);
    if (rc != 0 && rc != INT8UNKNOWN)
    {
        char buffer[32];
        /**
        ** Cut'n'paste from outint8() in output.c - lazy!
        ** JL 2003-04-07: Believe it or not, ifx_int8toasc() is defined
        ** not to null terminate the damn string!  See 5423.pdf (ESQL/C
        ** Programmers Reference Manual for IDS 9.21!  (Puke!)
        ** It also blank pads to full length.
        */
        ifx_int8toasc(&i8, buffer, sizeof(buffer));
        buffer[byleng(buffer, sizeof(buffer)-1)] = '\0';
        fprintf(fp, "%s%s = %s", pad, "SERIAL8", buffer);
        did_serial8 = 1;
    }
#endif /* ESQLC_IUSTYPES */

#ifdef ESQLC_BIGINT
    if (did_serial8 == 0)
    {
        bigint bi;
        ifx_getbigserial(&bi);
        if (bi != 0)
            fprintf(fp, "%s%s = %" PRId_ixInt8, pad, "BIGSERIAL", bi);
    }
#endif /* ESQLC_BIGINT */

    putc(' ', fp);
    fflush(fp);
}

static void print_sqlca(size_t nerrd, const CodeName *errd, size_t nwarn, const CodeName *warn)
{
    if (sqlca.sqlcode == 0 || sqlca.sqlcode == SQLNOTFOUND)
    {
        FILE *fp = ctxt_error();
        size_t    i;
        const char *pad = "";
        for (i = 0; i < nerrd; i++)
        {
            int j = errd[i].code;
            if (j < 0)
                j = -j;
            /* If index is negative, print unconditionally */
            if (errd[i].code < 0 || sqlca.sqlerrd[j] != 0)
            {
                fprintf(fp, "%s%s = %" PRId_ixInt4, pad, errd[i].name, sqlca.sqlerrd[j]);
                pad = ", ";
            }
        }
        if (sqlca.sqlwarn.sqlwarn0 == 'W' && nwarn > 0)
        {
            const char  *cptr = &sqlca.sqlwarn.sqlwarn0;
            pad = "";
            fprintf(fp, "%s(Warnings: ", (nerrd == 0) ? "" : " ");
            for (i = 0; i < nwarn; i++)
            {
                if (cptr[warn[i].code] == 'W')
                {
                    fprintf(fp, "%s%s", pad, warn[i].name);
                    pad = ", ";
                }
            }
            fprintf(fp, ")");
        }
        putc('\n', fp);
        fflush(fp);
    }
}

static void sql_sqlca(int stmt)
{
    static const CodeName errd[] =
    {
        { -2, "Rows processed" },
        {  5, "Last ROWID" },
    };
    static const CodeName warn[] =
    {
        { 1, "column truncated" },
        { 2, "aggregate is null" },
        { 3, "wrong number of host variables" },
        { 4, "no conditional clause" },
        { 5, "Non-ANSI (ISO) syntax" },
        { 6, "data fragment skipped during query processing" },
    };
    fprintf(ctxt_error(), "%s: ", sql_stmttype(stmt));
    if (stmt == SQ_INSERT && sqlca.sqlcode == 0)
        print_serials();
    print_sqlca(DIM(errd), errd, DIM(warn), warn);
}

void sql_dbinfo(void)
{
    static const CodeName warn[] =
    {
        { 1, "database has transactions" },
        { 2, "database is MODE ANSI" },
        { 3, "database is not SE" },
        { 4, "FLOAT is synonym for DECIAL" },
        { 6, "database server is HDR secondary (readonly)" },
        { 7, "client DB_LOCALE does not match database locale" },
    };
    print_sqlca(0, 0, DIM(warn), warn);
}

/* Execute an ordinary (non-select) SQL statement */
static void     sql_execute(int stmt_code)
{
    if (stmt_code == SQ_LDINSERT)
        sqlca.sqlcode = -836;
    else
    {
/*
 *         EXEC SQL EXECUTE s_statement;
 */
#line 497 "sqlread-sqlstmt.ec"
  {
#line 497 "sqlread-sqlstmt.ec"
  sqli_exec(ESQLINTVERSION, sqli_curs_locate(ESQLINTVERSION, _Cn1, 769), (ifx_sqlda_t *)0, (char *)0, (struct value *)0, (ifx_sqlda_t *)0, (char *)0, (struct value *)0, 0);
#line 497 "sqlread-sqlstmt.ec"
  }
    }
    if (sqlca.sqlcode < 0)
        sql_error();
    else if (ctxt_getverbose() == OP_ON)
        sql_sqlca(stmt_code);
}

/* Print data from a SELECT or EXECUTE PROCEDURE/FUNCTION statement */
static void     sql_select(Sqlda *idesc, int stmt_code)
{
    char           *buffer;
    long            count;
    long            qlimit;
    int             state = 0;
    long            status;

    /* Must print types from unmodified sqlda structure */
    if (print_header(idesc) != 0)
        return;
    qlimit = ctxt_getqlimit();

    /* Allocate space for SQLDA structure */
    buffer = (char *)sql_describe(idesc);   /*=C++=*/
    jb_register(buffer);

    /* This is a one-cycle loop that simplifies error handling */
    do
    {
        /* Fetch and print data */
/*
 *         EXEC SQL DECLARE c_statement CURSOR FOR s_statement;
 */
#line 527 "sqlread-sqlstmt.ec"
  {
#line 527 "sqlread-sqlstmt.ec"
  sqli_curs_decl_dynm(ESQLINTVERSION, sqli_curs_locate(ESQLINTVERSION, _Cn2, 512), _Cn2, sqli_curs_locate(ESQLINTVERSION, _Cn1, 513), 0, 0);
#line 527 "sqlread-sqlstmt.ec"
  }
        if (sqlca.sqlcode < 0)
            break;
        state = 1;

/*
 *         EXEC SQL OPEN c_statement;
 */
#line 532 "sqlread-sqlstmt.ec"
  {
#line 532 "sqlread-sqlstmt.ec"
  sqli_curs_open(ESQLINTVERSION, sqli_curs_locate(ESQLINTVERSION, _Cn2, 768), (ifx_sqlda_t *)0, (char *)0, (struct value *)0, 0, 0);
#line 532 "sqlread-sqlstmt.ec"
  }
        if (sqlca.sqlcode < 0)
            break;
        state = 2;

        count = 0;
        while (sqlca.sqlcode == 0)
        {
/*
 *             EXEC SQL FETCH c_statement USING DESCRIPTOR idesc;
 */
#line 540 "sqlread-sqlstmt.ec"
  {
#line 540 "sqlread-sqlstmt.ec"
  static _FetchSpec _FS0 = { 0, 1, 0 };
  sqli_curs_fetch(ESQLINTVERSION, sqli_curs_locate(ESQLINTVERSION, _Cn2, 768), (ifx_sqlda_t *)0, idesc, (char *)0, &_FS0);
#line 540 "sqlread-sqlstmt.ec"
  }
            if (sqlca.sqlcode != 0)
                break;
            if (qlimit > 0 && count++ >= qlimit)
            {
                fflush(ctxt_output());
                break;
            }
            if (print_record(idesc) != 0)
                break;
        }
        fflush(ctxt_output());
        if (sqlca.sqlcode != 0)
            break;

    } while (0);

    /* Output XML trailer record */
    print_trailer();

    /* Check for I/O error - damage already reported */
    if (ferror(ctxt_output()) != 0)
        sqlca.sqlcode = 0;

    if (sqlca.sqlcode < 0)
        sql_error();
    else if (sqlca.sqlcode == SQLNOTFOUND && ctxt_getverbose() == OP_ON)
        sql_sqlca(stmt_code);

    status = 0;
    switch (state)
    {
    case 2:
/*
 *         EXEC SQL CLOSE c_statement;
 */
#line 573 "sqlread-sqlstmt.ec"
  {
#line 573 "sqlread-sqlstmt.ec"
  sqli_curs_close(ESQLINTVERSION, sqli_curs_locate(ESQLINTVERSION, _Cn2, 768));
#line 573 "sqlread-sqlstmt.ec"
  }
        if (status == 0 && sqlca.sqlcode < 0)
            status = sqlca.sqlcode;
        /*FALLTHROUGH*/
    case 1:
/*
 *         EXEC SQL FREE c_statement;
 */
#line 578 "sqlread-sqlstmt.ec"
  {
#line 578 "sqlread-sqlstmt.ec"
  sqli_curs_free(ESQLINTVERSION, sqli_curs_locate(ESQLINTVERSION, _Cn2, 770));
#line 578 "sqlread-sqlstmt.ec"
  }
        if (status == 0 && sqlca.sqlcode < 0)
            status = sqlca.sqlcode;
        /*FALLTHROUGH*/
    default:
        break;
    }
    sqlca.sqlcode = status;

    if (sqlca.sqlcode < 0)
        sql_error();
    jb_unregister(buffer);
    FREE(buffer);
}

#ifdef SQLREAD

static int sqlread_allowed(int stmt_code)
{
    static int readonly_cmds[] =
    {
#ifdef SQ_SETENV
        SQ_SETENV,
#endif  /* SQ_SETENV */
#ifdef SQ_STMT_CACHE
        SQ_STMT_CACHE,
#endif  /* SQ_STMT_CACHE */
        SQ_SETROLE,
#ifdef SQ_SETDAC
        SQ_SETDAC,
#endif  /* SQ_SETDAC */
#ifdef SQ_PDQPRIORITY
        SQ_PDQPRIORITY,
#endif  /* SQ_PDQPRIORITY */
#ifdef SQ_SETDATASKIP
        SQ_SETDATASKIP,
#endif  /* SQ_SETDATASKIP */
        SQ_DBGFILE,
        SQ_OPTIM,
        SQ_EXPLAIN,
        SQ_SETLOG,
        SQ_ISOLATE,
        SQ_WAITFOR,
        SQ_SELINTO,
        SQ_SELECT,
    };
    size_t i;
    for (i = 0; i < DIM(readonly_cmds); i++)
    {
        if (stmt_code == readonly_cmds[i])
            return(1);
    }
    return(0);
}
#endif /* SQLREAD */

/* Execute internal or SQL command */
void sql_command(char *str)
{
/*
 *     EXEC SQL BEGIN DECLARE SECTION;
 */
#line 637 "sqlread-sqlstmt.ec"
#line 638 "sqlread-sqlstmt.ec"
  char *cmd;
/*
 *     EXEC SQL END DECLARE SECTION;
 */
#line 639 "sqlread-sqlstmt.ec"

    Sqlda *idesc;
    int    stmt_code;

    cmd = str;

    /* This is a one-cycle loop that simplifies error handling */
    do
    {
        /* Compile RDSQL statement */
/*
 *         EXEC SQL PREPARE s_statement FROM :cmd;
 */
#line 649 "sqlread-sqlstmt.ec"
  {
#line 649 "sqlread-sqlstmt.ec"
  sqli_prep(ESQLINTVERSION, _Cn1, cmd,(ifx_literal_t *)0, (ifx_namelist_t *)0, -1, 0, 0); 
#line 649 "sqlread-sqlstmt.ec"
  }
        if (sqlca.sqlcode < 0)
            break;

/*
 *         EXEC SQL DESCRIBE s_statement INTO idesc;
 */
#line 653 "sqlread-sqlstmt.ec"
  {
#line 653 "sqlread-sqlstmt.ec"
  sqli_describe_stmt(ESQLINTVERSION, sqli_curs_locate(ESQLINTVERSION, _Cn1, 769), &idesc, 0);
#line 653 "sqlread-sqlstmt.ec"
  }
        if (sqlca.sqlcode < 0)
            break;

        stmt_code = sqlca.sqlcode;
        if (stmt_code == 0)
        {
            stmt_code = SQ_SELECT;
            sql_select(idesc, stmt_code);
        }
#ifdef SQLREAD
        else if (sqlread_allowed(stmt_code))
            sql_execute(stmt_code);
        else
            cmd_resume(E_CMDNOTALLOWED, sql_stmttype(stmt_code));
#else
#ifdef SQ_EXECPROC
        /* Handle stored procedures which return data as SELECT statements */
        else if (stmt_code == SQ_EXECPROC && idesc->sqld > 0)
            sql_select(idesc, stmt_code);
#endif  /* SQ_EXECPROC */
        else
            sql_execute(stmt_code);
#endif /* SQLREAD */
        sqlca.sqlcode = 0;

/*
 *         EXEC SQL FREE s_statement;
 */
#line 679 "sqlread-sqlstmt.ec"
  {
#line 679 "sqlread-sqlstmt.ec"
  sqli_curs_free(ESQLINTVERSION, sqli_curs_locate(ESQLINTVERSION, _Cn1, 770));
#line 679 "sqlread-sqlstmt.ec"
  }
        if (sqlca.sqlcode < 0)
            break;

        free(idesc);
    } while (0);

    if (sqlca.sqlcode < 0)
        sql_error();
}

/* Process command line database connection (-d, -u, -p options). */
/* ESQL/C 5.x and earlier does not support username and password. */
/* Create appropriate CONNECT or DATABASE statement and execute   */
/* using the normal do_command() mechanism to clock it or log it. */
/* BUG: does not escape embedded quotes in user name and password */
void sql_dbconnect(char *dbase, char *uname, char *upass)
{
    char stmt[356] = "";    /* KLUDGE: 2*128 for dbase@server plus flab */

#ifdef ESQLC_CONNECT
    static const char wct[] = "WITH CONCURRENT TRANSACTIONS";

    if (uname != 0 && upass != 0)
        esnprintf(stmt, sizeof(stmt), "CONNECT TO '%s' USER '%s' PASSWORD '%s' %s",
                dbase, uname, upass, wct);
    else if (uname != 0)
        esnprintf(stmt, sizeof(stmt), "CONNECT TO '%s' USER '%s' %s",
                dbase, uname, wct);
    else
        esnprintf(stmt, sizeof(stmt), "CONNECT TO '%s' %s", dbase, wct);
#else
    esnprintf(stmt, sizeof(stmt), "DATABASE '%s'", dbase);
#endif /* ESQLC_CONNECT */

    do_command(stmt);
    scribble_buffer(stmt, sizeof(stmt));
}

/* List available database names */
void sql_dbnames(void)
{
    int             ndbs;
    int             i;
    char           *dbsname[MAXDBS + 1];
    char            dbsarea[FASIZ];

    if ((i = sqgetdbs(&ndbs, dbsname, MAXDBS, dbsarea, FASIZ)) != 0)
    {
        sqlca.sqlcode = i;
        sql_error();
    }
    else
    {
        for (i = 0; i < ndbs; ++i)
            fprintf(ctxt_output(), "%2d %s\n", i + 1, dbsname[i]);
    }
}

#line 736 "sqlread-sqlstmt.ec"
