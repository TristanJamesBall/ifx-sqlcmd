/*
@(#)File:           $RCSfile: context.h,v $
@(#)Version:        $Revision: 2015.1 $
@(#)Last changed:   $Date: 2015/11/09 01:37:52 $
@(#)Purpose:        Context Handling for SQLCMD
@(#)Author:         J Leffler
@(#)Copyright:      (C) JLSS 1992-93,1997-2005,2008,2011,2015
@(#)Product:        SQLCMD Version 90.02 (2016-07-28)
*/

#ifndef CONTEXT_H
#define CONTEXT_H

#ifdef MAIN_PROGRAM
#ifndef lint
/* Prevent over-aggressive optimizers from eliminating ID string */
extern const char jlss_id_context_h[];
const char jlss_id_context_h[] = "@(#)$Id: context.h,v 2015.1 2015/11/09 01:37:52 jleffler Exp $";
#endif /* lint */
#endif /* MAIN_PROGRAM */

/* -- Type Definitions */

/* Change array op_list[] in context.c when you change this list */
enum Operator
{
    OP_NONE,
    OP_OFF,
    OP_ON,
    OP_POP,
    OP_PUSH,
    OP_QUOTE,
    OP_SELECT,
    OP_TOGGLE,
    OP_UNLOAD,
    OP_FIXED,
    OP_FIXSEP,
    OP_FIXDEL,
    OP_CSV,
    OP_DB2,
    OP_XML,
    OP_HTML,
    OP_BLOCK,
	OP_MARKDOWN
};
typedef enum Operator Operator;

/* LOAD and UNLOAD information */
/* No significance to 91, or 101, except they are less boring than 1 */
enum LoadDest { LD_UNKNOWN, LD_PIPE = 91, LD_FILE };
typedef enum LoadDest LoadDest;

enum FileMode { FM_UNKNOWN, FM_CREATE = 101, FM_APPEND };
typedef enum FileMode FileMode;

/* -- Declarations */

extern void     ctxt_print(FILE *fp);
extern void     ctxt_init(void);
extern void     ctxt_free(void);
extern void     ctxt_recover(void);

extern char    *ctxt_loadfile(void);/* Name of load data file */
extern char    *ctxt_infile(void);  /* Name of input (SQL commands) file */
extern char    *ctxt_outfile(void); /* Name of output file */
extern char    *ctxt_errfile(void); /* Name of error file */
extern FILE    *ctxt_input(void);   /* Stream of input file */
extern FILE    *ctxt_output(void);  /* Stream of output file */
extern FILE    *ctxt_error(void);   /* Stream of error file */

extern int      ctxt_setloadinput(const char *str, LoadDest dest, FileMode mode);
extern int      ctxt_setunloadoutput(const char *str, LoadDest dest, FileMode mode);

extern int      ctxt_setinput(const char *fn);
extern int      ctxt_setoutput(const char *fn);
extern int      ctxt_seterror(const char *fn);
extern void     ctxt_newinput(FILE *fp, const char *fn);
extern void     ctxt_newoutput(FILE *fp, const char *fn);
extern void     ctxt_newerror(FILE *fp, const char *fn);

extern void     ctxt_newcontext(void);
extern void     ctxt_endcontext(void);

extern void     ctxt_setblobdir(const char *dir);
extern void     ctxt_setbmark(const char *op);
extern void     ctxt_setcontinue(const char *op);
extern void     ctxt_setdate(const char *fmt);
extern void     ctxt_setdelim(const char *delim);
extern void     ctxt_seteor(const char *eor);
extern void     ctxt_setescape(const char *escape);
extern void     ctxt_setformat(const char *op);
extern void     ctxt_setheading(const char *op);
extern void     ctxt_sethexmode(const char *op);
extern void     ctxt_sethistory(const char *op);
extern void     ctxt_setibase(const char *tag);
extern void     ctxt_setqlimit(const char *str);
extern void     ctxt_setquote(const char *quote);
extern void     ctxt_setsilence(const char *op);
extern void     ctxt_setsingle(const char *op);
extern void     ctxt_setskipsize(const char *op);
extern void     ctxt_settrace(const char *op);
extern void     ctxt_settransize(const char *str);
extern void     ctxt_settypes(const char *op);
extern void     ctxt_setverbose(const char *tag);
extern void     ctxt_setxmlheadertag(const char *tag);
extern void     ctxt_setxmlrecordtag(const char *tag);
extern void     ctxt_setxmlrecsettag(const char *tag);

extern char    *ctxt_getblobdir(void);
extern char    *ctxt_getdate(void);
extern char    *ctxt_geteor(void);
extern char    *ctxt_getxmlheadertag(void);
extern char    *ctxt_getxmlrecordtag(void);
extern char    *ctxt_getxmlrecsettag(void);
extern int      ctxt_getbmark(void);
extern int      ctxt_getcontinue(void);
extern int      ctxt_getdelim(void);
extern int      ctxt_getescape(void);
extern int      ctxt_getformat(void);
extern int      ctxt_getheading(void);
extern int      ctxt_gethexmode(void);
extern int      ctxt_gethistory(void);
extern int      ctxt_getibase(void);
extern int      ctxt_getqlimit(void);
extern int      ctxt_getquote(void);
extern int      ctxt_getsilence(void);
extern int      ctxt_getsingle(void);
extern int      ctxt_getskipsize(void);
extern int      ctxt_gettrace(void);
extern int      ctxt_gettransize(void);
extern int      ctxt_gettypes(void);
extern int      ctxt_getverbose(void);

/* Line numbering is managed internally - API not exposed to users */
extern void     ctxt_setlinenum(size_t linenum);
extern size_t   ctxt_getlinenum(void);

extern const char op_off[];
extern const char op_on[];
extern const char op_pop[];
extern const char op_push[];
extern const char op_toggle[];
extern const char op_unload[];
extern const char op_block[];

#endif /* CONTEXT_H */
