/*
 * freelist.h
 */
struct F_LIST {			/* 0 item keeps track of free/used counts */
	int free;			/* Storage is item+1 */
	int used;
};

/*
 * Free-list manipulation primitives
 *
 * A free list removal is as follows: Take used item, add to end of free list
 *   take item at end of used list and place in empty used list slot.  Then
 *   decrement used count and increment free count.
 * A free list addition is as follows: Take the last free item and add it to
 *   the end of the used list.  Increment used count, decrement free count.
 */
#define FLIST(x,size) struct F_LIST x[size+1]
#define FLIST_REM(n,x) n[++n[0].free].free=n[x+1].used;n[x+1].used=n[n[0].used--].used
#define FLIST_ADD(n) (n[++n[0].used].used=n[n[0].free--].free)
#define FLIST_INIT(n) flist_init(&n[0],sizeof(n)/sizeof(struct F_LIST))
void flist_init(struct F_LIST *f,int c);
