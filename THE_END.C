/*
 * the_end.c
 * Draw The End on the missile command screen
 */

#include "stdlib.h"
#include "mem.h"
#include "game.h"
#include "pixel.h"
#include "dos.h"

/*
 * Coordinates 1-8 are of the final edge of the explosion
 */
#define END_WIDTH (Y_SIZE/2-1)
#define END_XC	X_SIZE/2
#define END_YC	Y_SIZE/2
static struct GR_COORD end_coord[9]= {
	END_XC,END_YC, /* Centre */
	END_XC,END_YC-END_WIDTH,
	END_XC-END_WIDTH*2/3,END_YC-END_WIDTH*2/3,
	END_XC-END_WIDTH,END_YC,
	END_XC-END_WIDTH*2/3,END_YC+END_WIDTH*2/3,
	END_XC,END_YC+END_WIDTH,
	END_XC+END_WIDTH*2/3,END_YC+END_WIDTH*2/3,
	END_XC+END_WIDTH,END_YC,
	END_XC+END_WIDTH*2/3,END_YC-END_WIDTH*2/3,
};

/* 3x3 description of THE END line segments */
static struct GR_COORD end_let[] = {
	0,0,2,0,1,0,1,2,-1,-1,					/* T */
	0,0,0,2,0,1,2,1,2,0,2,2,-1,-1,			/* H */
	0,0,0,2,0,0,2,0,0,1,1,1,0,2,2,2,-1,-1,	/* E */
	0,0,0,2,0,0,2,0,0,1,1,1,0,2,2,2,-1,-1,	/* E */
	0,0,0,2,0,0,2,2,2,0,2,2,-1,-1,          /* N */
	0,0,0,2,0,0,1,0,1,0,2,1,1,2,2,1,0,2,1,2,-1,-1 /* D */
};

/*
 * Draw the text for "THE END"
 */
static void draw_the_end(void)
{
	int i,j,xory;
	int sx,sy,scx,scy,w,cx,cy;
	int fx,fy,tx,ty;
	struct GR_COORD *p;

	p=&end_let[0];
	sx=-4, sy=-3;	/* Message start point */
	scx=16, scy=16;		/* scaling */
	cx=sx=sx*scx+X_SIZE/2;
	cy=sy=sy*scy+Y_SIZE/2;
	w=2;				/* Width of stroke */
	for (i=0;i<6;i++) {
		while (p->x!=-1) {
			fx=p->x*scx+cx;
			fy=p++->y*scy+cy;
			tx=p->x*scx+cx;
			ty=p++->y*scy+cy;
			if (fy==ty) xory=0;
			else if (fx==tx) xory=1;
			else xory=2;
			for (j=-w;j<=w;j++) {
				switch (xory) {
				case 1:
					draw_line(fx+j,fy-w,tx+j,ty+w,CT_FMISS);
					break;
				case 0:
					draw_line(fx-w,fy+j,tx+w,ty+j,CT_FMISS);
					break;
				case 2:
					draw_line(fx,fy+j,tx,ty+j,CT_FMISS);
					break;
				}
			}
		}
		++p;
		if (i==2) cx=sx, cy=sy+4*scy;
		else cx+=3*scx;
		gdelay(30);
	}
}

/*
 * Draw the "the_end" screen
 * Current limitations: To damn slow, no synchronization is performed
 */
void the_end(void)
{
	struct GR_LINE edge[8];
	struct GR_COORD last,new, *b, *e;
	int i,flag,state,phase,update;

	nosound();
	init_mode(0x13);
	pal_setup(9);
/*	pal[CT_BACK][0]=63;
	pal[CT_BACK][1]=0;
	pal[CT_BACK][2]=40;   Fuchsia color requested by Gail */
	memmove(&pal[CT_FMISS][0],pal_color[C_BLUE],3);
	init_pal(&pal[0][0],0,CT_FMISS+1);
	for (state=0;state<2;state++) {
		memset(edge,0,sizeof(edge));
		for (i=0;i<8;i++) {				/* Set up the paths to the corners */
			if (state) {
				e=&edge[i].src;
				b=&edge[i].dst;
				edge[i].color=CT_BACK;
			} else {
				e=&edge[i].dst;
				b=&edge[i].src;
				edge[i].color=CT_HOT;
			}
			b->x=END_XC;
			b->y=END_YC;
			e->x=end_coord[i+1].x;
			e->y=end_coord[i+1].y;
			init_line(&edge[i]);
			if (i&1) edge[i].phase.cnt=edge[i].phase.max=3;
			else edge[i].phase.cnt=edge[i].phase.max=2;
		}
		flag=1;
		phase=0;
		update=1;
		while (flag) {
			if (++phase==4) {
				sync();
				phase=0;
			}
			flag=0;
			if (update) {
				last.x=edge[7].cur.x;
				last.y=edge[7].cur.y;
				for (i=0;i<8;i++) {
					draw_line(last.x,last.y,new.x=edge[i].cur.x,new.y=edge[i].cur.y,edge[i].color);
					draw_line(last.x+1,last.y,new.x+1,new.y,edge[i].color);
					last.x=new.x;
					last.y=new.y;
				}
			}
			update=0;
			for (i=0;i<8;i++) {
				pal_cycle();
				if (edge[i].cnt) {
					flag=1;
					if (!--edge[i].phase.cnt) {
						update=1;
						inc_line(&edge[i]);
						edge[i].phase.cnt=edge[i].phase.max;
					}
				}
			}

		}
		if (state) return;
		draw_the_end();
		gdelay(200);
	}
}

