/*
 * demo.c
 * Introductory stuff
 */
#include "stdio.h"
#include "conio.h"
#include "mem.h"
#include "stdlib.h"
#include "dos.h"
#include "pixel.h"
#include "high.h"
#include "game.h"
#include "oprintf.h"
#include "timer.h"

/*
 * Scrolled message (No boast -- It took months the first time round in '81-'82)
 * Final tally (excluding stock C header files):
 * [mouse / VGA version and demo]
 *   50 hours programming and "testing" (10 hours for demo)
 *   Lines of code:   New    Existing  Total
 *         H          340          23    363
 *         C         2182         316   2498
 *        ASM         216         395    611
 *       Total       2738         734   3472
 * -jok, Jan. 26, 1992
 */
char smsg[] = "Mousal Command written by Jeff Kesner Jan 19 to 26, 1992.\
  Press left button for 1 player, right for 2, middle to\
 exit.  Use A, S, or D to replace mouse buttons, if desired.   ";
struct {
	char *cp;
	char *msg;
	int scpos;
	int phase;
} smsg_cb = { smsg,smsg,0,0};

char arrow_bm[] = {		/* DEFEND CITIES arrow bitmaps */
	0,0,0,2,2,2,2,2,0,0,0,
	0,0,0,2,2,2,2,2,0,0,0,
	0,0,0,2,2,2,2,2,0,0,0,
	2,2,2,2,2,2,2,2,2,2,2,
	0,2,2,2,2,2,2,2,2,2,0,
	0,0,2,2,2,2,2,2,2,0,0,
	0,0,0,2,2,2,2,2,0,0,0,
	0,0,0,0,2,2,2,0,0,0,0,
	0,0,0,0,0,2,0,0,0,0,0
};
struct GR_OBJECT arrow = {11,9,arrow_bm,0,0,0};

struct DTARG dtarg;

/*
 * Display the last line scrolling message
 */
void show_msg(void)
{
	if (smsg_cb.phase++<15) return;
	smsg_cb.phase=0;
	if (smsg_cb.scpos++<4) {
		scroll_info(25);
		return;
	}
	smsg_cb.scpos=0;
	scrmsg(oprintf("%c",*smsg_cb.cp++),38,24,CT_FMISS);
	if (!*smsg_cb.cp) smsg_cb.cp=smsg_cb.msg;
}

/*
 * Display the DEFEND CITIES arrows
 */
void arrows(int func)
{
	int i;
	static int phase=0;

	if (func) {
		if (++phase&255) return;
		if ((phase&256)==0) phase=0;
	}
	for (i=0;i<6;i++) {
		if (!player[0].city[i]) continue;
		arrow.x=cities[i].x-(arrow.width>>1);
		arrow.y=150;
		sprite(&arrow,!(phase&256) && func);
	}
}


/*
 * Launch an ABM for the demo player
 */
void handle_dmis(int kind)
{
	int base, xref;
	struct MOUSE_STAT m;

	xref=kind>>5;
	base=(kind>>3)&3;
	if (dtarg.targs[xref].ignore) return; /* Missile is gone already */
	m.x=cross_hair.x;
	m.y=cross_hair.y;
	handle_mis(base,&m);
}

/*
 * Move a missile along over a specified time range
 */
void inc_time(long time, struct GR_LINE *l)
{
	long li,lc;
	struct GR_LINE l1;

	memmove(&l1,l,sizeof(l1));
	init_line(&l1);
	li=time/l1.phase.max;
	lc=li*l1.inc.numer/l1.inc.denom;
	if (l1.xory) l->cur.x+=li*l1.dir.x, l->cur.y+=lc*l1.dir.y;
	else l->cur.x+=lc*l1.dir.x, l->cur.y+=li*l1.dir.y;
}

/*
 * Kill a target
 * moving at rate phase along a path parallel to (sx,sy)->(dx,dy)
 * currently located at position cx,cy
 */
int demo_kill(int cx, int cy,int phase, int sx, int sy, int dx, int dy, int kind, int flip)
{
	struct GR_LINE enemy,cross,orig;
	struct GR_COORD c;
	int i,it,time,ptime,base,iy;

	enemy.src.x=sx, enemy.src.y=sy, enemy.dst.x=dx, enemy.dst.y=dy;
	init_line(&enemy);
	enemy.cur.x=cx, enemy.cur.y=cy;
	enemy.phase.max=phase;
	i=(dtarg.last+MAX_LINES-1)%MAX_LINES;
	if (dtarg.count) {
		c.x=dtarg.targs[i].x;
		c.y=dtarg.targs[i].y;
	} else {
		c.x=cross_hair.x;
		c.y=cross_hair.y;
	}

	/*
	 * First, look forward to next possible launch
	 */
	inc_time(dtarg.time,&enemy);

	if (kind==LN_SMART || 1) {	/* Allow for peculiarities with bitmap */
		enemy.cur.x-=3;			/* of cross-hair */
		enemy.cur.y+=1;

	}
	memmove(&orig,&enemy,sizeof(orig));
	time=0,ptime=1;
	nosound();
	/*
	 * Iterate to find best shot.
	 * Should be an o(log(n)) calculation, if I'm looking at this right
	 */
	for (it=0;it<12 && ptime!=time ;it++) {
		ptime=time;
		memmove(&enemy,&orig,sizeof(enemy));
		inc_time(time,&enemy);
		iy=enemy.cur.y;
		base=(enemy.cur.x<X_SIZE/2)?0:2;
		if (dtarg.base[1]<dtarg.base[base] && player[2].mbase[1]) {
			if (!flip) base=1;
		} else if (flip) base=1;
		if (!player[2].mbase[base]) base=2-base;
		if (!player[2].mbase[base]) base=1; /* Only middle base left */
		cross.src.x=bases[base].x;         /* Allow for ABM travel */
		cross.src.y=bases[base].y;
		cross.dst.x=enemy.cur.x;
		cross.dst.y=enemy.cur.y;
		init_line(&cross);
		time=cross.cnt*((base==1)?1:2);
		cross.src.x=c.x;
		cross.src.y=c.y;
		init_line(&cross);
		time+=cross.cnt*dtarg.cspeed;		/* Allow for cross hair motion */
		if (kind!=LN_SMART) {
			if (enemy.cur.y<X_SIZE/2)
				time+=((50-sdat.speed)>>2)*enemy.phase.max;
			else
				time+=4*enemy.phase.max;
		}
	}
	cross.dst.x=enemy.cur.x;
	cross.dst.y=enemy.cur.y;
	if (kind==LN_SMART && iy>mlimit.y.high)
		return -1; /* No hope */
	if (kind!=LN_SMART && iy>mlimit.y.high+3)
		return -1;
	init_line(&cross);
	dtarg.time+=cross.cnt*dtarg.cspeed;
	i=dtarg.last;
	dtarg.last=(i+1)%MAX_LINES;
	++dtarg.count;
	dtarg.targs[i].x=enemy.cur.x, dtarg.targs[i].y=enemy.cur.y;
	dtarg.targs[i].base=base;
	dtarg.targs[i].ignore=0;
	dtarg.base[base]++;
	return (dtarg.time+time)/enemy.phase.max+1;
}

/*
 * Return the indicated lines target x point
 */
int tarx(int t)
{
	int tr;
	tr=msl[t].target&7;
	if (tr<3) return bases[tr].x;
	return cities[tr-3].x;
}

/*
 * Sort prospective ICBMs by the x coordinate of what they are targetted on
 */
int comp_targ(const void *e1,const void *e2)
{
	int *i1, *i2;
	i1=(int *)e1, i2=(int *)e2;
	return(tarx(*i1)-tarx(*i2));
}

/*
 * Target something for the demo player
 */
void demo_targ(void)
{
	struct GR_LINE *tl;
	int i,x,y,b,rc,flip;
	int pro[MAX_LINES], pc;

	if (--dtarg.time<0) dtarg.time=0;
	if (sdat.air_demo) return;
	if (dtarg.count) {
		--dtarg.count;
		i=dtarg.first;
		x=dtarg.targs[i].x;
		y=dtarg.targs[i].y;
		b=dtarg.targs[i].base;
		dtarg.first=(dtarg.first+1)%MAX_LINES;
		if (x<mlimit.x.low) x=mlimit.x.low;
		if (y<mlimit.y.low) y=mlimit.y.low;
		if (x>mlimit.x.high) x=mlimit.x.high;
		if (y>mlimit.y.high) y=mlimit.y.high;
		new_line(cross_hair.x,cross_hair.y,x,y,dtarg.cspeed,LN_DEMO+(b<<3)+(i<<5),-1);
		++sdat.air_demo;
	}
	if (dtarg.phase--) return;
	dtarg.phase=30;
	for (pc=i=0;i<flist_line[0].used;i++) {
		tl=&msl[flist_line[i+1].used];
		if (tl->demo) continue;
		if (tl->kind==LN_EMISS || tl->kind==LN_SMART) {
			if (tl->target<3 && player[2].mbase[tl->target]>1)
				pro[pc++]=flist_line[i+1].used;
			if (tl->target>=3 && player[2].city[tl->target-3])
				pro[pc++]=flist_line[i+1].used;
		}
	}
	if (pc>1) qsort(pro,pc,sizeof(int),comp_targ);
	for (i=0;i<pc;i++) {
		tl=&msl[pro[i]];
		for (rc=-1,flip=0;rc==-1 && flip<2;++flip)
			rc=demo_kill(tl->cur.x,tl->cur.y,tl->phase.max,tl->src.x,tl->src.y,
				tl->dst.x, tl->dst.y, tl->kind, flip);
		if (rc>0) {
			++tl->demo;
			tl->demo_time=rc; /* Expected time of demolition */
			tl->demo_list=(dtarg.last-1+MAX_LINES)%MAX_LINES;
		}
	}
	if (sat.plot_dat && !sat.demo) {
		if (sat.x<X_SIZE/4 || sat.x>X_SIZE*3/4) return;
		sat.demo=1;
		demo_kill(sat.x+5*sat.dir,sat.y+5,SAT_PHASE_MAX,3+sat.dir,0,3+sat.dir*2,0,LN_EMISS,0);
	}
}

char *imsg[] = {
	"ICBM", "CONTRAIL", "YOUR SIGHT", "MISSILE BASE", "MIRV", "CRUISE MISSILES",
	"STRATEGIC BOMBER", "ORBITTING WEAPONS PLATFORM", "SMART BOMB"
};

/*
 * Play a demo set with instructions
 */
void update_instructions(void)
{
	if (game.active) --game.active;
	sound_update();
	offense();
	demo_targ();
	update_lines();
	update_circles();
	update_bomber();
}

/*
 * Draw the points screen
 */
void draw_points(void)
{
	static char *pmsgs[6][2] = {
		"OBJECT", "SCORE", "SMART BOMB", "125", "BOMBER", "100",
		"ICBM", "25", "CITY", "100", "ABM", "5"
	};
	int i;

	smart_bomb.x=156;
	smart_bomb.y=41;
	sprite(&smart_bomb,1);

	for (i=20;i>0;i--)
		plot_sat(bomber_dat,182-i,52,-1);

	for (i=20;i>0;i--)
		plot_sat(sat_dat,153-i,52,-1);


	draw_line(156,67,164,77,CT_EMISS);
	pixel(164,77,CT_HOT);

	draw_city(6,1,cityb);

	base_mis.x=159;
	base_mis.y=107;
	sprite(&base_mis,1);

	for (i=0;i<6;i++) {
		scrmsg(oprintf("%+17s",pmsgs[i][0]),0,(i<<1)+3,(i==0)?CT_EMISS:CT_FMISS);
		scrmsg(pmsgs[i][1],23,(i<<1)+3,(i==0)?CT_EMISS:CT_FMISS);
	}

}

/*
 * Amuse the proles who can't cough up the $0.50 to play the game
 */
void intro(void)
{
	struct MOUSE_STAT m;
	int i,phase,state,c;

	game.players=0;
	game.demo_set=0;
	init_player(2);
	pal_setup(game.last_set>>1);
	draw_set(0);
	phase=1;
	state=-1;

	memset(&m,0,sizeof(m));
	smsg_cb.cp=smsg_cb.msg;
	while (!game.players || game.active) {
		if (--phase==0) {
			phase=3000;
			scr_clear();
			switch(++state) {
			case 0:	/* High score screen */
				disp_high();
			case 3:
				nosound();
				init_player(2);
				draw_set(2);
				scrmsg(oprintf("DEFEND CITIES"),13,16,CT_FMISS);
				if (state) disp_high();
				state=0;
				scrmsg(oprintf("BONUS CITY FOR %ld",game.bonus),10,13,CT_EMISS);
				break;
			case 1: /* Points screen */
				draw_points();
				scrmsg(oprintf("DEFEND CITIES"),13,16,CT_FMISS);
				break;
			case 2: /* Demo screen */
				init_player(2);
				player[game.cur_player=2].set=game.demo_set++;
				init_set(2);
				memset(&dtarg,0,sizeof(dtarg));
				dtarg.cspeed=sdat.speed/6;
				if (!dtarg.cspeed) dtarg.cspeed=1;	/* Set cross hair speed */
				cross_hair.x=X_SIZE/2;
				cross_hair.y=Y_SIZE/2;
				break;
			}
		}
		sync();
		pal_cycle();
		if (game.flags.mouse) mouse_stat(&m);
		show_msg();
		if (game.active || state==2) {
			state=2;
			if (game.active) phase=game.active;
			update_instructions();
		} else
			arrows(1);

		c=scankey();
		if (c) c|=32;
		if (c=='o' && state!=2) {
			phase=1;
			state=1;
		}
		if (m.but.bit.m || c=='s'|| c==' ') {
			init_mode(game.start_mode);
			SetClkRate(0);
			Clk_uninstall();
			init_mode(game.start_mode);
			nosound();
			exit(0);
		}
		if (m.but.bit.l || c=='a') {
			target.noattack=target.fasttrack=1;
			game.players=1;
		}
		if (m.but.bit.r || c=='d') {
			target.noattack=target.fasttrack=1;
			game.players=2;
		}
	}
	for (i=0;i<game.players;i++) init_player(i);
	scr_clear();
}
