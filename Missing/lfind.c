/*
@(#)File:           $RCSfile: lfind.c,v $
@(#)Version:        $Revision: 1.5 $
@(#)Last changed:   $Date: 2011/11/28 04:39:39 $
@(#)Purpose:        General Linear Search Algorithm
@(#)Author:         J Leffler
@(#)Copyright:      (C) JLSS 1998,2003,2005,2011
@(#)Product:        SQLCMD Version 90.02 (2016-07-28)
*/

/*TABSTOP=4*/

#include <search.h>

#define CONST_CAST(t,v)	((t)(v))	/* Simulate C++ const_cast<t>(v) */

#ifndef lint
/* Prevent over-aggressive optimizers from eliminating ID string */
const char jlss_id_lfind_c[] = "@(#)$Id: lfind.c,v 1.5 2011/11/28 04:39:39 jleffler Exp $";
#endif /* lint */

void   *lfind(
	const void *item,	/* Pointer to item sought	 */
	const void *table,	/* Pointer to start of table	 */
	size_t *tablesize,	/* Number of items in table	 */
	size_t entrysize,	/* Size of one entry in table	 */
	int (*compare)(const void *, const void *))	/* Comparison function		 */
{
	size_t  i;
	size_t  n = *tablesize;
	int     result;
	const void *entry;
	const char *base = table;

	for (i = 0; i < n; i++)
	{
		entry = base + i * entrysize;
		result = (*compare)(item, entry);
		if (result == 0)
			return(CONST_CAST(void *, entry));		/* Found */
	}
	return(0);					/* Not found */
}

#ifdef TEST

#include <stdio.h>
#include <assert.h>
#include <string.h>

static char *array[] =
{
	"abcd",
	"bc",
	"abce",
	"def"
};

static char *lookup[] =
{
	"a",
	"abce",
	"bc",
	"def",
	"abcd",
	"e",
	"abcdef",
	"ba",
	"caliph"
};

#define DIM(x) (sizeof(x)/sizeof(*(x)))

static int qs_compare(const void *p1, const void *p2)
{
	const char *const *s1 = (const char *const *)p1;
	const char *const *s2 = (const char *const *)p2;
	return(strcmp(*s1, *s2));
}

int main(void)
{
	size_t i;
	size_t n = DIM(array);
	char **s;

	printf("Data:\n");
	for (i = 0; i < DIM(array); i++)
		printf("%lu: %s\n", (unsigned long)i, array[i]);

	printf("Searching:\n");
	for (i = 0; i < DIM(lookup); i++)
	{
		s = lfind(&lookup[i], array, &n, sizeof(char *), qs_compare);
		if (s == 0)
			printf("<<%s>> not found.\n", lookup[i]);
		else
			printf("<<%s>> found at index %td (<<%s>>)\n", lookup[i], s - array, *s);
	}

	n = 0;
	s = lfind(&lookup[0], array, &n, sizeof(char *), qs_compare);
	assert(s == 0);
	s = lfind(&lookup[0], 0, &n, sizeof(char *), qs_compare);
	assert(s == 0);

	printf("OK\n");

	return 0;
}

#endif /* TEST */
