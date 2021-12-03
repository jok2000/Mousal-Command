/*
 * grprim.c
 * Graphics primitives
 */

#include "pixel.h"
#include "stdlib.h"
#include "stdio.h"
#include "mem.h"
#include "game.h"
#include "dos.h"

/*
 * Wipe a line out from its current position to its source
 */
void clear_line(struct GR_LINE *l)
{
	int cnt;

	cnt=l->cnt;
	init_line(l);
	while (l->cnt>=cnt) {
		pixel(l->cur.x,l->cur.y,0);
		pixel(l->cur.x+1,l->cur.y,0);
		inc_line(l);
	}
}

/*
 * Plain old lineto, but done with our own line drawer
 */
void draw_line(int sx,int sy,int dx,int dy,int color)
{
	struct GR_LINE l;

	l.src.x=sx;
	l.src.y=sy;
	l.dst.x=dx;
	l.dst.y=dy;
	l.color=color;
	init_line(&l);
	while (l.cnt>0) {
		pixel(l.cur.x,l.cur.y,color);
		inc_line(&l);
	}
}

/*
 * Advance a line pointer system by one pixel
 */
void inc_line(struct GR_LINE *l)
{
	l->cnt--;
	if ((l->cur_frac+=l->inc.numer) > l->inc.denom) {
			l->cur.x+=l->dir.x;
			l->cur.y+=l->dir.y;
			l->cur_frac-=l->inc.denom;
	} else {
		if (l->xory) l->cur.x+=l->dir.x;
		else l->cur.y+=l->dir.y;
	}
}

/*
 * Initialize a line pointer system
 */
void init_line(struct GR_LINE *l)
{
	int dx,dy,sdx,sdy;

	l->phase.cnt=0;
	dx=abs(sdx=l->dst.x-(l->cur.x=l->src.x));	/* Compute deltas */
	dy=abs(sdy=l->dst.y-(l->cur.y=l->src.y));
	if (dx>dy) {				/* Set up count and +1 direction */
		l->xory=1;
		l->cnt=dx;
		l->inc.numer=dy;
		l->inc.denom=dx;
	} else {
		l->xory=0;
		l->cnt=dy;
		l->inc.numer=dx;		/* Fractional increment */
		l->inc.denom=dy;
	}
	l->cur_frac=l->inc.denom/2;	/* Current fraction numerator start value */

	if (sdx>0) l->dir.x=1;
	else if (sdx<0) l->dir.x=-1;	/* Set x direction */
	else l->dir.x=0;

	if (sdy>0) l->dir.y=1;
	else if (sdy<0) l->dir.y=-1; /* Set y direction */
	else l->dir.y=0;
}

/*
 * Plot an object on the screen (not really a sprite routine anymore)
 */
int sprite(struct GR_OBJECT *g, int state)
{
	int cx,cy,mx,my,dead,p,b;
	static char *bp, *bg;

	dead=0;
	bg=g->back;
	bp=g->bitmap;
	my=g->y+g->height;
	mx=g->x+g->width;

	/*
	 * First handle placing a sprite on the screen
	 */
	if (state) {
		for (cy=g->y;cy<my;cy++) {
			for (cx=g->x;cx<mx;cx++) {
				if ((b=*bp++)!=0) {
					if (bg)
						*bg++=p=pixel(cx,cy,b);
					else
						p=pixel(cx,cy,b);
					dead|=(p==CT_EXPLO);
				}
			}
		}
		return(dead);
	}
	for (cy=g->y;cy<my;cy++) {
		for (cx=g->x;cx<mx;cx++) {
			if ((b=*bp++)!=0) {
				if (bg) {
					p=pixel(cx,cy,*bg);
					if (b==CT_CROSS && p!=CT_CROSS && p!=*bg) pixel(cx,cy,p);
						/* Stupid hack for cross hair */
					++bg;
				} else
					p=pixel(cx,cy,0);
				dead|=(p==CT_EXPLO);

			}
		}
	}
	return(dead);
}

/*
 * Integer circle drawer
 */
void draw_circle(int x,int y, int r, int c)
{
	extern void pixc(int x, int y, int c); /* Pixel check and plot */
	int dx,dy,osq,sq,xsq,ysq,dif,t;

	if (r<2) {
		pixc(x,y,c);
		return;
	}
	dy=0;
	ysq=1;
	osq=sq=r*r;
	dx=r-1;
	xsq=r+r-1;
	while (dy<=dx) {
		pixc(x+dx,y+dy,c); /* Do all 8 simple symetries */
		pixc(x+dy,y+dx,c);
		pixc(x-dx,y-dy,c);
		pixc(x-dy,y-dx,c);
		pixc(x-dx,y+dy,c);
		pixc(x-dy,y+dx,c);
		pixc(x+dx,y-dy,c);
		pixc(x+dy,y-dx,c);
		sq+=(t=ysq);		/* sq is now sum of squares */
		ysq+=2;
		++dy;				/* Move along y direction */
		dif=((sq-osq)<<1) -xsq; /* Difference between sqaure of lengths */
		if (dif > 0) {
			sq-=xsq;
			xsq-=2;
			--dx;
			if (dif >= t) { /* This if THICKENS the circle for filling */
				ysq=t;
				sq-=ysq;
				--dy;
			}
		}
	}
}

/*
 * Horizontally scroll a text line on the VGA screen
 */
void scroll_info(int ln)
{
	unsigned int i,dp;

	dp=(ln-1)*8;
	for (i=0;i<8;i++) {
		pixel(0,dp+i,CT_EMISS);
		pixel(1,dp+i,CT_EMISS);
	}
	dp*=X_SIZE;
	movedata(SCR_SEG,dp+2,SCR_SEG,dp,X_SIZE*8-2);
	pixel(X_SIZE-1,Y_SIZE-1,CT_EMISS);
	pixel(X_SIZE-2,Y_SIZE-1,CT_EMISS);
}

/*
 * Set overscan (not currently used)
 */
void pal_over(int color)
{
	union REGS rin, rout;

	rin.x.ax=0x1001;
	rin.x.bx=color<<8;
	int86(0x10, &rin, &rout);
}

/*
 * Display a message on the screen
 */
void scrmsg(char *str,int x,int y,int color)
{
	union REGS rin, rout;

	rin.h.ah=2;
	rin.h.bh=0;
	rin.h.dh=(unsigned char)y;
	rin.h.dl=(unsigned char)x;
	int86(0x10, &rin, &rout);
	while (*str) {
		rin.h.ah=0x0e;		/* Write char */
		rin.h.al=*str++;	/* Mode 0, BL is attribute */
		rin.h.bl=color;
		rin.h.bh=0;			/* Page number */
		int86(0x10, &rin, &rout);
		if (y==24) char_cor(x++<<3);
	}
}

/*
 * Clear the playing area
 */
void scr_clear(void)
{
	static char aline[X_SIZE];
	int i;

	for (i=10;i<Y_SIZE-30;i++)
		movedata(_DS,(unsigned)aline,0xa000,i*X_SIZE,X_SIZE);
}

char pal_color[8][3] = {   /* Standard color set for missile command */
	0,0,0,		/* Black */
	63,0,0,		/* Red */
	0,63,0,     /* Green */
	63,63,0,    /* Yellow */
	0,0,63,     /* Blue */
	63,0,63,    /* Magenta */
	0,63,63,    /* Cyan */
	63,63,63	/* White */
};

char pal[256][3]; /* The game's color palette */
