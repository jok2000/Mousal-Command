/*
 * freelist.c
 */

#include "freelist.h"

/*
 * Initialize a free list
 */
void flist_init(struct F_LIST *f,int c)
{
	f[0].free=c-1;
	f[0].used=0;
	while (--c)	f[c+1].free=c;
}
