/*
@(#)File:           $RCSfile: sqlconn.h,v $
@(#)Version:        $Revision: 2010.1 $
@(#)Last changed:   $Date: 2010/12/18 22:45:11 $
@(#)Purpose:        Handle SQL connection management operations
@(#)Author:         J Leffler
@(#)Copyright:      (C) JLSS 2005-06,2010
@(#)Product:        SQLCMD Version 90.02 (2016-07-28)
*/

/*TABSTOP=4*/

#ifndef SQLCONN_H
#define SQLCONN_H

#ifdef  __cplusplus
extern "C" {
#endif

#include "esqlc.h"

#ifdef MAIN_PROGRAM
#ifndef lint
/* Prevent over-aggressive optimizers from eliminating ID string */
const char jlss_id_sqlconn_h[] = "@(#)$Id: sqlconn.h,v 2010.1 2010/12/18 22:45:11 jleffler Exp $";
#endif /* lint */
#endif /* MAIN_PROGRAM */

enum { MAX_SERVER_LENGTH = 128 };       /* In 9.30 */
enum { MAX_DBNAME_LENGTH = 128 };       /* In 9.30 */
enum { MAX_DBASE_LENGTH  = (MAX_SERVER_LENGTH+MAX_DBNAME_LENGTH+sizeof("''@''")) };

/*
** CONN_NONE    - no connection name was specified.
** CONN_STRING  - connection name is string
** CONN_DEFAULT - connection is default
** CONN_CURRENT - disconnect current (set connection current dormant)
** CONN_ALL     - disconnect all
*/
enum ConnType { CONN_NONE, CONN_STRING, CONN_DEFAULT, CONN_CURRENT, CONN_ALL };
typedef enum ConnType ConnType;

struct ConnInfo
{
    char    *database;      /* Connection dbase@server */
    char    *username;      /* Connection Username */
    char    *password;      /* Connection Password */
    char    *connname;      /* Connection name */
    ixUint1  ctype;         /* Connection type */
    ixUint1  wct;           /* Connection with concurrent TX */
    ixUint1  trusted;       /* Connection for Trusted Context */
    ixUint1  mode_ansi;     /* DBase is MODE ANSI (vs regular) */
    ixUint1  logged;        /* DBase is Logged (vs UnLogged) */
    ixUint1  online;        /* DBase is OnLine (vs SE) */
    ixUint1  current;       /* Connection is current */
};
typedef struct ConnInfo ConnInfo;

extern void sql_connect(ConnInfo *conn);
extern void sql_disconn(ConnInfo *conn);
extern void sql_setconn(ConnInfo *conn);

#ifdef  __cplusplus
}
#endif

#endif /* SQLCONN_H */
