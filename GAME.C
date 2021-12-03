/*
 * game.c
 * Game-specific functionality
 */

#include "stdio.h"
#include "conio.h"
#include "mem.h"
#include "stdlib.h"
#include "time.h"
#include "dos.h"
#include "pixel.h"
#include "high.h"
#include "game.h"
#include "oprintf.h"
#include "timer.h"

struct GAME game;			/* Game control record */
struct PLAYER player[3];	/* Player control reocrd */
int land_limit[X_SIZE];		/* Land background info */
struct SAT sat;				/* Satellite control info */
struct SOUND_CB sound_cb;	/* Sound effect control info */
struct TARGET target;		/* Targetting control info */

enum SOUNDS {SND_NONE, SND_WARNING, SND_BOMBER, SND_SMART, SND_BOOM, SND_OUT,
	SND_LOW, SND_NUMBER
};
struct SOUND_CB sound_desc[SND_NUMBER] = {
	1, 850, 850, -32, -32, 30, 30, 1, 5, 5,  3,	/* Warning */
	2,  80,  80,  -1,   1, 10, 10, 0, 3, 3,  2,	/* Bomber sound */
	3, 450, 450,   2,   2, 15, 15, 2, 2, 2, 40, /* Smart bomb */
	4,  52,  52,   2,  -1, 15, 30, 0, 2, 2,  2, /* Boom */
	5,1500,1500, -32, -32, 30, 30, 1, 1, 1,  1,	/* Missiles out */
	6,1000,1000, -32, -32, 30, 30, 1, 2, 2,  3	/* Missiles low */
};

char set_colors[10][CT_NUM] = {
	C_BLACK,  C_BLUE,  C_RED,   C_CYAN, C_WHITE, C_BLUE,
	C_BLACK,  C_RED,   C_GREEN, C_CYAN, C_WHITE, C_RED,
	C_BLACK,  C_YELLOW,C_BLUE, C_CYAN, C_WHITE,  C_YELLOW,
	C_BLACK,  C_BLUE,  C_YELLOW, C_CYAN, C_WHITE,C_BLUE,
	C_CYAN,   C_BLACK, C_RED, C_MAGENTA, C_WHITE,C_BLACK,
	C_WHITE,  C_BLUE,  C_BLACK, C_MAGENTA, C_WHITE,C_BLUE,
	C_MAGENTA,C_GREEN, C_BLACK, C_CYAN, C_WHITE, C_GREEN,
	C_YELLOW, C_BLUE,  C_BLACK, C_CYAN, C_WHITE, C_BLUE,
	C_WHITE,  C_BLUE,  C_BLACK, C_CYAN, C_WHITE, C_BLUE,
	C_RED,    C_BLACK, C_YELLOW, C_CYAN, C_WHITE, C_BLACK
};

/* Missile base positions */
struct GR_COORD bases[3] = { 20,172, 160,172, 299,172};

/* Game city position */
struct GR_COORD cities[6] = {
	123,188, 194,186, 88,181, 229,184, 55,183, 267,186
};

/* Bonus city display positions */
struct GR_COORD cityb[7] = {
	117,91, 142,91, 167,91, 192,91, 216,91, 242,91, 160,92
};

/* Offset of player's missiles in the bases */
struct GR_COORD mis_dis[10] = {
	0,2, -3,6, 3,6, -6,10, 0,10,
	6,10,-9,14, -3,14, 3,14, 9,14
};

/* Bit map of ABM */
char base_mis_bm[] = {
	0,1,0,
	0,1,0,
	1,0,1,
};
char base_mis_bg[sizeof(base_mis_bm)];
struct GR_OBJECT base_mis = {3,3,base_mis_bm,base_mis_bg,0,0};
char base_mis_bg2[sizeof(base_mis_bm)];
struct GR_OBJECT base_mis2 = {3,3,base_mis_bm,base_mis_bg2,0,0};

enum { DD_BACKUP=-3, DD_COLOR, DD_END};
/* Strategic bomber draw instructions */
char bomber_dat[] = {
	12,11,10,8,7,6,1,0,6,8,9,11,12,DD_BACKUP,3,19,18,17,DD_COLOR,0,
	DD_BACKUP,0,
	13,13,12,11,11,10,20,20,10,11,11,12,13,DD_BACKUP,3,20,20,20,DD_END
};

/* Orbitting weapons platform draw instructions */
char sat_dat[] = {
	DD_BACKUP,1,1,2,3,4,3,3,3,3,4,3,2,1,DD_BACKUP,1,12,11,10,9,10,10,10,
	10,9,10,11,12,DD_BACKUP,3,5,DD_BACKUP,10,5,
	DD_BACKUP,6,6,6,DD_BACKUP,6,DD_COLOR,CT_HOT,
	4,4,DD_BACKUP,6,8,8,DD_BACKUP,0,0,DD_BACKUP,0,13,DD_BACKUP,13,0,
	DD_BACKUP,13,13,DD_COLOR,0,DD_BACKUP,0,1,2,3,4,10,11,11,11,11,10,4,3,2,1,DD_BACKUP,0,
	14,13,12,11,DD_BACKUP,3,9,DD_BACKUP,10,9,DD_BACKUP,10,11,12,13,14,DD_END
};

struct SDAT sdat;	/* Enemy attack characteristics working copy */
struct {			/* Enemy attack characteristics for each set (const) */
	char silo_missiles;
	char ships;
	char silo_smarts;
	char speed;
} set_data[] = {
	12,0,0,50,  16,2,0,30, 20,3,0,19, 16,4,0,16, 20,4,0,14,
	16,3,1,12,   20,4,3,11,  12,3,2,10,  16,4,4,9, 20,5,5,8,
	25,5,6,7,   23,4,6,6
};

char cross_hair_bm[] = { /* The cross hair bit map */
	0,0,0,5,0,0,0,
	0,0,0,5,0,0,0,
	0,0,0,5,0,0,0,
	5,5,5,5,5,5,5,
	0,0,0,5,0,0,0,
	0,0,0,5,0,0,0,
	0,0,0,5,0,0,0
};
char cross_hair_back[sizeof(cross_hair_bm)];
struct GR_OBJECT cross_hair = { 7,7,cross_hair_bm,cross_hair_back,160,100};

char smart_bm[] = { /* The smart bomb's bit map */
	0,0,0,2,0,0,0,
	0,0,2,2,2,0,0,
	0,2,2,3,2,2,0,
	2,2,3,3,3,2,2,
	0,2,2,3,2,2,0,
	0,0,2,2,2,0,0,
	0,0,0,2,0,0,0
};
/* char smart_bg[sizeof(smart_bm)]; */
struct GR_OBJECT smart_bomb = {7,7,smart_bm,0,20,20};

char city_dat[] = {	/* City building height data */
	2,4,6,4,9,6,9,9,5,6,10,9,7,5,7,4,5,4,3,2,-1,
	0,0,2,2,3,4,3,2,4,2,4,6,3,2,3,3,2,-1
};

char ex_bm[] = {	/* Targetted ABM destination marker bitmap */
	3,0,0,0,3,
	0,3,0,3,0,
	0,0,3,0,0,
	0,3,0,3,0,
	3,0,0,0,3
};
char ex_bg[sizeof(ex_bm)];
struct GR_OBJECT ex = {5,5,ex_bm,ex_bg,40,40};

/*
 * SMART, ICBM, ABM, MIRV and CRUISE control information
 */
struct GR_LINE msl[MAX_LINES];
FLIST(flist_line,MAX_LINES);

/*
 * Explosion control information
 */
#define MAX_CIRCS 40
struct GR_CIRC msc[MAX_CIRCS];
FLIST(flist_circ,MAX_CIRCS);

struct MLIMIT mlimit;

/*
 * Synchronize game motion to a 300/sec interrupt heart beat
 */
void sync(void)
{
	while (!game.sync);
	game.sync=0;
}

/*
 * Interrupt routine call this function
 */
unsigned int TimeOut(void)
{
	game.sync=1;
	return(1);
}

/*
 * Draw a strategic bomber or orbitting weapons platform
 * Object is assumed to be moving 1 pixel at a time in "dir" direction
 */
int plot_sat(char *dat, int x, int y, int dir)
{
	int c,dead,flag,cx,cy;

	dead=0;
	c=CT_EMISS;
	cx=x;
	cy=y;
	for (flag=1;flag;) {
		if (*dat>=0) {
			if (dir>0) cx=x+*dat++;
			else cx=x-*dat++;
			if (cx>=0 && cx<X_SIZE)
				dead|=pixel(cx,cy,c)==CT_EXPLO;
			++cy;
		} else {
			switch (*dat++) {
			case DD_BACKUP:
				cy=y+*dat++;
				break;
			case DD_COLOR:
				c=*dat++;
				break;
			case DD_END:
				flag=0;
				break;
			}
		}
	}
	return dead;
}

/*
 * Do the explosion color cycling
 */
void pal_cycle(void)
{
	static int exp_state=0, phase=0;
	static char exp_colors[4][3] = {63,63,63, 63,63,0, 63,0,63, 0,63,63};

	if (++phase<12) return;
	phase=0;
	if (++exp_state>3) exp_state=0;
	memmove(&pal[CT_HOT][0],exp_colors[exp_state],3);
	memmove(&pal[CT_EXPLO][0],exp_colors[exp_state^3],3);
	init_pal(&pal[CT_HOT][0],CT_HOT,CT_NUM);
}

/*
 * Setup the pallet for the current set
 */
void pal_setup(int set)
{
	int i;

	set%=10;
	for (i=0;i<CT_NUM;i++)
		memmove(pal[i],pal_color[set_colors[set][i]],3);
	for (i=0;i<3;i++) if (pal[0][i]) pal[0][i]=50;
	init_pal(&pal[0][0],0,CT_NUM);
}

/*
 * Put something useful in the pallet at the start
 */
void pal_init(void)
{
	int i;

	for (i=0;i<256;i++)
		pal[i][0]=pal[i][1]=pal[i][2]=i&63;
	memmove(&pal[0][0],&pal_color[0][0],sizeof(pal_color));
	init_pal(&pal[0][0],0,256);
}

/*
 * Basic game setup information
 */
void init(void)
{
	time_t t;
	time(&t);
	srand((unsigned)t);	/* Set random number generator seed */
	game.start_mode=init_mode(0x13);
						/* Graphics screen */
	pal_init();  		/* Set default color scheme */
	pal_setup(0);		/* Set 0 color scheme */
	if (!mouse_reset()) game.flags.mouse=0;		/* Reset the mouse */
	mlimit.x.low=0, mlimit.x.high=X_SIZE-cross_hair.width;
	mlimit.y.low=8, mlimit.y.high=Y_SIZE-51;
	if (game.flags.mouse) mouse_limits(mlimit.x.low, mlimit.x.high, mlimit.y.low, mlimit.y.high);
	FLIST_INIT(flist_line);
	FLIST_INIT(flist_circ);
	game.sync=1;
	Clk_install();
	SetClkRate(game.sync_cnt);
	StartTimeOut(1);
	init_player(0);
}

/*
 * Start a player's game by initializing PLAYER structure
 */
void init_player(int p)
{
	int j;

	memset(&player[p],0,sizeof(struct PLAYER));
	player[p].cities=game.cities;	/* Set up number of cities */
	for (j=0;j<player[p].cities;j++)
		player[p].city[j]=1;		/* Assign cities */
}

/*
 * Move a smart bomb
 */
void inc_smart(struct GR_LINE *l)
{
	int left,right,centre,x,y,xo,yo;

	if (sound_cb.kind<SND_SMART) init_sound(SND_SMART);
	x=l->cur.x;
	y=l->cur.y;
	left=right=0;
	for (xo=4,yo=-4;xo>2;xo=(yo<=0)?xo+2:xo-2) {
		if ((x>xo) && (pixel(x-xo,y+yo,-1)==CT_EXPLO)) ++left;
		if ((x<X_SIZE-xo) && (pixel(x+xo,y+yo,-1)==CT_EXPLO)) ++right;
		yo+=2;
	}
	centre=pixel(x,y+5,-1)==CT_EXPLO;
	if (y>12 && y<Y_SIZE-50 && (right||centre||left)) { /* Dodge */
		if (centre) {
			l->cur.y--;
			if (!left && !right)
				if (l->cur.x+=l->dir.x);
		} else
			l->cur.y++;
		if (left) l->cur.x++;
		if (right) l->cur.x--;
		l->src.x=l->cur.x;
		l->src.y=l->cur.y;
		init_line(l);
	} else
		inc_line(l);
}

/*
 * Continue drawing missile tracks
 */
void update_lines(void)
{
	int i,dead,kind,dmiss;
	struct GR_LINE *l;

	dmiss=-1;
	for (i=0;i<flist_line[0].used;i++) {
		game.active=ACTIVITY;
		l=&msl[flist_line[i+1].used];
		if (++l->phase.cnt<l->phase.max) continue; /* Current phase? */
		l->phase.cnt=0;
		if (target.fasttrack) {
			l->phase.cnt=l->phase.max-2;
			if (l->phase.cnt<1) l->phase.cnt=1;
		}
		dead=0;
		kind=1;	/* Deadly explosion default */
		if (l->demo) if (!l->demo_time--) l->demo=0;
		switch (l->kind&7) {
		case LN_SMART:
			smart_bomb.x=l->cur.x-3;
			smart_bomb.y=l->cur.y-3;
			dead=sprite(&smart_bomb,0);
			inc_smart(l);
			smart_bomb.x=l->cur.x-3;
			smart_bomb.y=l->cur.y-3;
			if (l->cnt) dead|=sprite(&smart_bomb,1);
			else {
				kill_target(l->target);
				dead=1;
				kind=0;
			}
			break;
		case LN_EMISS:
		case LN_FMISS:
			dead|=(pixel(l->cur.x+1,l->cur.y,l->color)==CT_EXPLO);
			dead|=(pixel(l->cur.x,l->cur.y,l->color)==CT_EXPLO);
			inc_line(l);
			if (l->cnt) {
				dead|=pixel(l->cur.x+1,l->cur.y,CT_HOT)==CT_EXPLO;
				dead|=pixel(l->cur.x,l->cur.y,CT_HOT)==CT_EXPLO;
			}
			if (l->kind==LN_FMISS || l->kind==LN_DEMO) dead=0;
			else if (!l->cnt) {
				kill_target(l->target);
				kind=0;
			}
			if (!l->cnt || dead) {
				clear_line(l);
				dead=1;
			}
			break;
		case LN_DEMO:
			cross_hair.x=l->cur.x;
			cross_hair.y=l->cur.y;
			sprite(&cross_hair,0);
			if (!l->cnt) dead=1;
			inc_line(l);
			cross_hair.x=l->cur.x;
			cross_hair.y=l->cur.y;
			sprite(&cross_hair,1);
		}
		if (dead) {
			FLIST_REM(flist_line,i);
			--i;
			if (kind && (l->kind&7)!=LN_DEMO) {
				new_circle(l->cur.x,l->cur.y,kind);
				free_target(l->target);
			}
			/* Indicate to demo targetter that this line was blown away */
			if (l->demo) dtarg.targs[l->demo_list].ignore=1;
			l->cnt=0;
			switch (l->kind&7) {
			case LN_FMISS:
				sdat.air_abm--;
				break;
			case LN_DEMO:
				sdat.air_demo--;
				dmiss=l->kind;
				break;
			case LN_EMISS:
				--sdat.air_missiles;
				if (kind) inc_score(25);
				init_sound(SND_BOOM);
				break;
			case LN_SMART:
				--sdat.air_smarts;
				if (kind) inc_score(125);
				init_sound(SND_BOOM);
				break;
			}
		}
	}
	if (dmiss!=-1) handle_dmis(dmiss);
}

/*
 * Update explosions
 */
void update_circles(void)
{
	int i,col,oact;
	struct GR_CIRC *c;

	for (oact=i=0;i<flist_circ[0].used;i++) {
		game.active=ACTIVITY;
		c=&msc[flist_circ[i+1].used];
		if (c->kind==1) oact=1; /* Any deadly explosions? */
		if (--c->phase) continue;
		c->phase=18;
		if (!(c->stage+=c->dir)) {
			FLIST_REM(flist_circ,i);
			--i;
			continue;
		}
		if (c->dir<0) col=CT_BACK;
		else {
			if (c->kind) col=CT_EXPLO;
			else col=CT_HOT;
		}
		draw_circle(c->x,c->y,c->stage,col);
		if (c->dir>0 && c->stage==18) {
			c->stage++;
			c->dir=-1;
		}
	}
	/*
	 * Put the game into fasttrack mode if the defenses are a lost cause
	 */
	if (!oact && !sdat.air_abm) {
		for (i=0;i<3;i++)
			if (player[game.cur_player].mbase[i]) break;
		if (i==3) target.fasttrack=1;
	}
}

/*
 * Start an explosion
 */
void new_circle(int x,int y,int kind)
{
	int i;
	struct GR_CIRC *c;

	if (!flist_circ[0].free) return;
	i=FLIST_ADD(flist_circ);
	c=&msc[i];
	c->stage=0;
	c->phase=1;
	c->dir=1;
	c->x=x;
	c->y=y;
	c->kind=kind;
}

/*
 * Start a missile going somewhere
 */
void new_line(int sx, int sy, int dx, int dy, int phase,int kind,int target)
{
	int i;
	struct GR_LINE *l;

	if (!flist_line[0].free) return;
	if (kind==LN_FMISS) {
		ex.x=dx-(ex.width>>1);
		ex.y=dy-(ex.height>>1);
		sprite(&ex,1);
	}
	i=FLIST_ADD(flist_line);
	l=&msl[i];
	l->src.x=sx;
	l->src.y=sy;
	l->dst.x=dx;
	l->dst.y=dy;
	l->phase.max=phase;
	switch(l->kind=kind) {
	case LN_EMISS:
		l->color=CT_EMISS;
		break;
	case LN_FMISS:
		l->color=CT_FMISS;
		break;
	}
	l->target=target;
	l->demo=0;
	init_line(l);
}

/*
 * Begin a sound effect
 */
void init_sound(int kind)
{
	if (!game.flags.sound) return;
	if (kind>0 && kind <SND_NUMBER) {
		memmove(&sound_cb,&sound_desc[kind-1],sizeof(struct SOUND_CB));
		sound(sound_cb.c_freq);
	} else
		nosound();
}

/*
 * Process sound effects
 */
void sound_update(void)
{
	if (!game.flags.sound) return;
	if (!sound_cb.kind) return;
	if (--sound_cb.c_phase) return;
	sound_cb.c_phase=sound_cb.s_phase;
	sound_cb.c_freq+=sound_cb.c_step;
	sound_cb.c_step+=sound_cb.step2;
	sound(sound_cb.c_freq);
	if (--sound_cb.c_step_cnt) return;
	sound_cb.c_step_cnt=sound_cb.s_step_cnt;
	sound_cb.c_freq=sound_cb.s_freq;
	sound_cb.c_step=sound_cb.s_step;
	if (--sound_cb.repeat) return;
	nosound();
	sound_cb.kind=0;
}

/*
 * Set up cities in city array
 */
void assign_cities(struct PLAYER *p)
{
	int existing, gaps, i, add, j, t;
	int where[6];

	for (existing=gaps=i=0;i<6;i++) {
		if (p->city[i]) ++existing;
		else where[gaps++]=i;
	}
	if (!gaps || p->cities<=existing) return;
	add=p->cities-existing;
	if (add>gaps) add=gaps;
	/*
	 * Shuffle the placing
	 */
	for (i=0;i<gaps;i++) {
		j=rand()%gaps;
		t=where[i], where[i]=where[j], where[j]=t;
	}
	for (i=0;i<add;i++)
		p->city[where[i]]=1;
}

/*
 * Play the game
 */
void play(void)
{
	int p,flag;
	extern void start_set(int p);

	for (flag=1;flag;) {
		flag=0;
		for (p=0;p<game.players;p++) {
			if (player[p].cities) {
				assign_cities(&player[p]);
				start_set(p);
				if (!player[p].cities) {
					scrmsg(oprintf("GAME OVER PLAYER %d",p+1),10,14,CT_EMISS);
					gdelay(333);
					scrmsg(oprintf("%32s",""),10,14,CT_BACK);
					check_high(player[p].score);
				} else {
					flag=1;
					player[p].set++;
				}
			}
		}
	}
}

/*
 * Draw city number c in the coordinate array cp, with state st
 */
void draw_city(int c,int st,struct GR_COORD *cp)
{
	int flag,x,y,co;
	char *s;

	flag=2;
	s=city_dat;
	x=cp[c].x-10;
	if (st) co=CT_FMISS;
	else co=CT_BACK;
	for (flag=2;flag;++x) {
		if ((y=*s++)==-1) {
			--flag;
			if (st) co=CT_HOT;
			x=cp[c].x-11;
			continue;
		}
		if (y) draw_line(x,cp[c].y+3,x,cp[c].y+3-y-1,co);
	}
}

/*
 * Draw the land
 */
void draw_land(void)
{
	int i;
	for (i=Y_SIZE;i>=Y_SIZE-9;i--)
		draw_line(0,i,X_SIZE-1,i,CT_EMISS);
	draw_line(160,Y_SIZE-10,X_SIZE-1,Y_SIZE-10,CT_EMISS);
	for (i=0;i<4;i++)
		draw_line(41,Y_SIZE-10-i,71,Y_SIZE-10-i,CT_EMISS);
	for (i=0;i<6;i++)
		draw_line(71,Y_SIZE-10-i,100,Y_SIZE-10-i,CT_EMISS);
	for (i=0;i<2;i++)
		draw_line(214,Y_SIZE-11-i,244,Y_SIZE-11-i,CT_EMISS);
}

/*
 * Draw the player's missile bases
 */
void draw_bases(void)
{
	static char base_dat[] = {13,14,15,17,18,19,21,23,24,24,23};
	int b,i;
	char c;

	for (b=0;b<3;b++) {
		for (i=0;i<11;i++) {
			c=base_dat[i]-2;
			draw_line(bases[b].x+20-i,Y_SIZE-9,bases[b].x+20-i,Y_SIZE-9-c,CT_EMISS);
			draw_line(bases[b].x-20+i,Y_SIZE-9,bases[b].x-20+i,Y_SIZE-9-c,CT_EMISS);
		}
		for (i=-9;i<10;i++)
			draw_line(bases[b].x+i,Y_SIZE-9,bases[b].x+i,Y_SIZE-28,CT_EMISS);
	}

}

/*
 * Explosion pixel limit calculations (called from GRPRIM.C)
 */
void pixc(int x,int y, int c)
{
	if (x<0 || x>=X_SIZE || y<8 || y>=Y_SIZE-8) return;
	if (!c && y>=land_limit[x])
		c=CT_EMISS;
	pixel(x,y,c);
}

/*
 * Set up the explosion redraw table
 */
void set_land_limit(void)
{
	int x,y;

	for (x=0;x<X_SIZE;x++) {
		land_limit[x]=Y_SIZE-35;
		for (y=Y_SIZE-35;y<Y_SIZE;y++) {
			if (pixel(x,y,-1)==0) continue;
			land_limit[x]=y;
			break;
		}
	}
}

/*
 * Draw the land, missile bases, cities, and missiles in bases
 */
void draw_set(int pl)
{
	struct PLAYER *p;
	int i,j;

	p=&player[pl];
	if (pl<2)
		draw_land();
	draw_bases();
	/*
	 * Draw missiles in bases
	 */
	for (i=0;i<3;i++) {
		for (j=0;j<10;j++) {
			base_mis.x=bases[i].x+mis_dis[j].x-1;
			base_mis.y=bases[i].y+mis_dis[j].y;
			sprite(&base_mis,1);
		}
	}
	/*
	 * Draw player's cities
	 */
	for (i=0;i<6;i++)
			draw_city(i,p->city[i],&cities[0]);
	/*
	 * Scores
	 */
	scrmsg(oprintf("%8ld%6s%8ld %-7.3s %8ld",player[1].score,"",hs[0].score,
		hs[0].inits,player[0].score),0,0,CT_FMISS);
	set_land_limit();
}

/*
 * Increment using the multiplier and then display the score
 */
void inc_score(int val)
{
	if (game.cur_player>1) return; /* Ignore demo player */
	val*=game.times;
	scrmsg(oprintf("%8ld",player[game.cur_player].score+=val),
		(game.cur_player==0)?31:0,0,CT_FMISS);
}

/*
 * Move satelite & bomber
 */
void update_bomber(void)
{
	if (!sat.plot_dat) return;
	game.active=ACTIVITY;
	if (--sat.phase) return;
	sat.phase=(target.fasttrack)?4:SAT_PHASE_MAX;
	sat.x+=sat.dir;
	if (sat.x<-20 || sat.x>X_SIZE+20) {
		if (sound_cb.kind==SND_BOMBER) init_sound(0);
		sat.plot_dat=NULL;
		return;
	}
	if (plot_sat(sat.plot_dat,sat.x,sat.y,-sat.dir)) {
		new_circle(sat.x-7*sat.dir,sat.y+6,1);
		inc_score(100);
		if (sound_cb.kind==SND_BOMBER) init_sound(SND_BOOM);
		sat.plot_dat=NULL;
		return;
	}
	if (sound_cb.kind<SND_BOMBER) init_sound(SND_BOMBER);
}

/*
 * Initialize targetting for this set
 */
void init_target(void)
{
	int i;

	memset(&target,0,sizeof(target));
	for (i=0;i<3;i++)
		target.targetted[i].kind=TR_BASE|i;

	for (i=0;i<6;i++) target.targetted[i+3].kind=TR_CITY|i;
}

/*
 * Take one attack off of the indexed target
 */
void free_target(int index)
{
	if (index<0) return;
	if (--target.targetted[index].cnt==0 &&
		(target.targetted[index].kind&TR_CITY) &&
		player[game.cur_player].city[index-3])
			target.lcity--;
}

/*
 * The enemy hit a target -- kill it
 */
void kill_target(int index)
{
	free_target(index);
	if (target.targetted[index].kind&TR_CITY) {
		new_circle(cities[index-3].x,cities[index-3].y,0);
		if (player[game.cur_player].city[index-3]) {
			target.taken++;
			if (target.targetted[index].cnt) target.lcity--;
			player[game.cur_player].city[index-3]=0;
			player[game.cur_player].cities--;
		}
	} else {
		new_circle(bases[index].x,bases[index].y+9,0);
		if (player[game.cur_player].mbase[index]) {
			player[game.cur_player].mbase[index]=0;
			if (game.cur_player<2)
				scrmsg(oprintf("OUT"),(bases[index].x>>3)-1,24,CT_FMISS);
		}
	}
}

/*
 * Target the object indicated
 */
void target_set(int index,struct TARGET_RET *tr)
{
	tr->index=index;
	target.targetted[index].tcnt++;
	if (!target.targetted[index].cnt++ &&
		(target.targetted[index].kind&TR_CITY) &&
		player[game.cur_player].city[index-3])
			target.lcity++;

	if (target.targetted[index].kind&TR_CITY) {
		tr->x=cities[index-3].x;
		tr->y=cities[index-3].y;
	} else {
		tr->x=bases[index].x;
		tr->y=bases[index].y;
	}
}

/*
 * Return alive/dead of target
 */
int worth(int index)
{
	if (index<3) return(player[game.cur_player].mbase[index]!=0);
	return (player[game.cur_player].city[index-3]);
}

/*
 * Find a target
 */
struct TARGET_RET *get_target(void)
{
	static struct TARGET_RET tr;
	int i,j,t,pt;
	static int p_target[9]; /* Cities we might want to add to the target list */

	/*
	 * First priority: city coords not currently under attack
	 */
	for (pt=0,i=3;i<9;i++) if (!target.targetted[i].cnt) p_target[pt++]=i;

	/* Shuffle the target list */
	if (pt>1) {
		for (i=0;i<pt;i++) {
			j=rand()%pt;
			t=p_target[i], p_target[i]=p_target[j], p_target[j]=t;
		}
	}
	/*
	 * Less than 3 live cities under attack?  Well, then attack one more
	 */
	if ((target.lcity+target.taken)<3 && pt) {
		for (i=0;i<pt;i++) { /* Find a live city to attack */
			if (player[game.cur_player].city[p_target[i]-3]) {
				target_set(p_target[i],&tr);
				return(&tr);
			}
		}
	}
	/*
	 * Well, that was a bust, make a new list
	 */
	for (pt=i=0;i<9;i++)
		if (i<3 || !player[game.cur_player].city[i-3])
			p_target[pt++]=i;

	/* Shuffle the target list */
	for (i=0;i<pt;i++) {
		j=rand()%pt;
		t=p_target[i], p_target[i]=p_target[j], p_target[j]=t;
	}
	/* Even out the attack */
	for (t=0,i=1;i<pt;i++)
		if (target.targetted[p_target[t]].tcnt>target.targetted[p_target[i]].tcnt)
			if (worth(p_target[t])<=worth(p_target[i])) t=i;
	target_set(p_target[t],&tr);
	return(&tr);
}

/*
 * Stage an attack
 */
void offense(void)
{
	int i,l,x,y,quit;
	struct TARGET_RET *tr;

	if (!target.lphase--) return; /* Attack on cue */
	target.lphase=(sdat.speed<<4)+1;

	if (sdat.wait--) return;	/* Programmed wait */
	sdat.wait=0;
	/*
	 * If we have inflicted maximum damage, do not attack
	 */
	quit=1;
	if (target.taken==3 || player[game.cur_player].cities==0) {
		for (i=0;i<3;i++)
			if (player[game.cur_player].mbase[i]) quit=0;
	} else
		quit=0;
	if ((target.noattack=quit)==1) return;
	/*
	 * First bring out the stategic bomber
	 *  or the orbiting weapons platform
	 */
	if (!sat.plot_dat && sdat.ships && (sdat.silo_missiles || sdat.silo_smarts
		|| sdat.air_missiles || sdat.air_smarts))
	{
		sdat.ships--;
		sat.demo=0;
		i=rand();
		sat.plot_dat=(i&512)?bomber_dat:sat_dat;
		sat.phase=17;
		if (i&256) {
			sat.dir=-1;
			sat.x=X_SIZE+20;
		} else {
			sat.dir=1;
			sat.x=-20;
		}
		sat.y=10+(i&63);
	}
	/*
	 * Check if launched missiles have reached the defense radar yet.
	 */
	if (target.llines || target.lsmarts) {
		tr=get_target();
		y=10, x=rand()%314+3;
		switch (target.where) {
		case L_SHIP:
			if (target.where==L_SHIP && sat.plot_dat && sat.x>40 && sat.x<280)
				y=sat.y+10, x=sat.dir*10+sat.x;
			break;
		case L_MIRV:
			x=target.mx, y=target.my;
			break;
		}
		if (target.llines) {
			l=LN_EMISS;
			--target.llines;
		} else {
			l=LN_SMART;
			--target.lsmarts;
		}
		new_line(x,y,tr->x,tr->y,sdat.speed,l,tr->index);
		return;
	}
	/*
	 * New wave?
	 */
	if (sdat.air_missiles<2 && sdat.air_smarts==0) {
		/*
		 * Now the missiles
		 */
		i=rand();
		target.where=L_TOP;
		y=11;
		/*
		 * Release the cruise missiles
		 */
		if (i&4) target.where=L_SHIP;
		/*
		 * Launch the smart bombs
		 */
		if (sdat.silo_smarts &&  ((i&3)==0 || sdat.silo_missiles==0)) {
			target.lsmarts=rand()&3;
			if (target.lsmarts>sdat.silo_smarts) target.lsmarts=sdat.silo_smarts;
			sdat.silo_smarts-=target.lsmarts;
			sdat.air_smarts+=target.lsmarts;
		} else {
			/*
			 * Launch the ICBMS
			 */
			target.llines=4+(rand()&3);
			if (target.llines>6) target.llines=6;
			if (target.llines>sdat.silo_missiles) target.llines=sdat.silo_missiles;
			/*
			 * Release the MIRVs
			 */
			if (sdat.air_missiles && !(i&48)) {
				for (i=0;i<flist_line[0].used;i++) {
					int x,y;
					l=flist_line[i+1].used;
					x=msl[l].cur.x, y=msl[l].cur.y;
					if (msl[l].kind==LN_EMISS && y<Y_SIZE/2 &&
						(x<X_SIZE/2 && y-10<x || x>X_SIZE/2 && y-10<X_SIZE-x))
					{
						target.mx=x;
						target.my=y;
						target.where=L_MIRV;
						break;
					}
				}
			}
			sdat.air_missiles+=target.llines;
			sdat.silo_missiles-=target.llines;
		}
		sdat.wave++;
		sdat.wait=sdat.speed;	/* Give the player a moment to recover */
		target.stage=0;
	}
}

/*
 * Fire a missile from the indicated base
 */
void handle_mis(int base,struct MOUSE_STAT *m)
{
	int mc;

	if (!player[game.cur_player].mbase[base]) {
		init_sound(SND_OUT);
		return;
	}
	mc=--player[game.cur_player].mbase[base];
	base_mis.x=bases[base].x+mis_dis[mc].x-1;
	base_mis.y=bases[base].y+mis_dis[mc].y;
	sprite(&base_mis,0);
	new_line(bases[base].x,bases[base].y,m->x+cross_hair.width/2,
		m->y+cross_hair.height/2,(base==1)?1:2,LN_FMISS,-1);
	sdat.air_abm++;
	if (mc==3) {
		init_sound(SND_LOW);
		if (game.cur_player<2)
			scrmsg(oprintf("LOW"),(bases[base].x>>3)-1,24,CT_FMISS);
	}
	if (mc==0 && game.cur_player<2)
		scrmsg(oprintf("OUT"),(bases[base].x>>3)-1,24,CT_FMISS);
}

/*
 * Conduct the defensive actions
 */
void defense(struct MOUSE_STAT *m)
{
	static struct MOUSE_STAT lm;
	int c;

	c=scankey();
	if (c) {
		switch (c|32) {
		case 'a':
		case ' ':
			handle_mis(0,m);
			break;
		case 's':
			handle_mis(1,m);
			break;
		case 'd':
			handle_mis(2,m);
			break;
		}
	}

	if (m->but.bit.l!=lm.but.bit.l && m->but.bit.l) handle_mis(0,m);
	if (m->but.bit.m!=lm.but.bit.m && m->but.bit.m) handle_mis(1,m);
	if (m->but.bit.r!=lm.but.bit.r && m->but.bit.r) handle_mis(2,m);
	memmove(&lm,m,sizeof(lm));
}

/*
 * Delay del game time units, but maintain pal_cycle()
 */
void gdelay(int del)
{
	while (del--) {
		sync();
		pal_cycle();
	}
}

/*
 * Initialize the set play
 */
void init_set(int p)
{
	int i,set;
	cross_hair.x=cross_hair.y=0;
	game.cur_player=p;
	game.prev_score=player[p].score; /* Save this score for bonus calc */
	game.last_set=set=player[p].set;
	pal_setup(set>>1);
	memset(&sdat,0,sizeof(sdat));
	init_target();
	/*
	 * Set data for enemy
	 */
	i=(set>9)?(set&1)+10:set;
	memmove(&sdat,&set_data[i],sizeof(set_data[0]));
	if (set>10) {	/* Give the poor sap a break once in a while */
		i=rand();
		sdat.silo_missiles-=i&3;
		if (i&4) sdat.silo_smarts-=1;
		if (i&8) --sdat.ships;
	}
	game.times=(player[p].set+2)&255; /* Mimic original set wrap around */
	if (game.times>11) game.times=6;	/* Max is 6x bonus points */
	else game.times>>=1;
	for (i=0;i<3;i++) player[p].mbase[i]=10; /* 10 missiles in each base */
	draw_set(p);
}

/*
 * Initialize and play one set
 */
void start_set(int p)
{
	struct MOUSE_STAT m;
	int i,j,clr;
	int base,city,cs,cc,ms,mc;

	memset(&m,0,sizeof(m));
	init_set(p);
	scrmsg(oprintf("GET READY PLAYER %d",p+1),10,8,CT_EMISS);
	clr=1;
	scrmsg(oprintf("%d X BONUS",game.times),15,10,CT_FMISS);
	init_sound(SND_WARNING);

	for (game.active=ACTIVITY;game.active;--game.active) {
		sync();
		pal_cycle();
		sound_update();
		if (game.flags.mouse) mouse_stat(&m);
		if (m.x!=cross_hair.x || m.y!=cross_hair.y) {
			sprite(&cross_hair,0);
			cross_hair.x=m.x;
			cross_hair.y=m.y;
			sprite(&cross_hair,1);
		}
		if (clr) {
			game.active=ACTIVITY;
			if (sound_cb.kind!=SND_WARNING) {
				scrmsg(oprintf("%32s",""),10,8,CT_BACK);
				scrmsg(oprintf("%9s",""),15,10,CT_BACK);
				clr=0;
			}
			continue;
		}
		offense();
		defense(&m);
		update_lines();
		update_circles();
		update_bomber();
	}
	init_sound(0);
	sprite(&cross_hair,0);
	/*
	 * Set over, do bonus points etc.
	 */
	scrmsg("BONUS POINTS",13,7,CT_EMISS);
	scrmsg("0",12,9,CT_FMISS);
	scrmsg("0",12,11,CT_FMISS);
	cs=cc=ms=mc=0;
	/*
	 * Bonus for left over missiles
	 */
	for (base=0;base<3;base++) {
		for (i=player[p].mbase[base];i>0;) {
			--i;
			if (game.flags.sound) sound(850);
			base_mis.x=bases[base].x+mis_dis[i].x-1;
			base_mis.y=bases[base].y+mis_dis[i].y;
			sprite(&base_mis,0);
			base_mis2.y=77;
			base_mis2.x=mc++*5+112;
			sprite(&base_mis2,1);
			scrmsg(oprintf("%4d",ms+=game.times*5),9,9,CT_FMISS);
			nosound();
			gdelay(30);
		}
	}
	/*
	 * Bonus for surviving cities
	 */
	for (city=0;city<6;city++) {
		if (player[p].city[city]) {
			if (game.flags.sound) sound(850);
			draw_city(city,0,&cities[0]);
			draw_city(cc++,1,&cityb[0]);
			scrmsg(oprintf("%4d",cs+=game.times*100),9,11,CT_FMISS);
			nosound();
			gdelay(90);
		}
	}
	player[p].score+=cs+ms;
	inc_score(0);
	/*
	 * Bonus city for points scored
	 */
	if (game.bonus) {
		if ((i=player[p].score/game.bonus-game.prev_score/game.bonus)!=0) {
			player[p].cities+=i;
			scrmsg("BONUS CITY",15,15,CT_FMISS);
			for (j=0;j<10;j++) {
				if (game.flags.sound) sound((rand()%1000)+100);
				gdelay((rand()%80)+20);
			}
			nosound();
		}
	}
	for (i=7;i<14;i+=2)
		scrmsg(oprintf("%38s",""),0,i,CT_BACK);
		scrmsg(oprintf("%38s",""),0,15,CT_BACK);
		scrmsg(oprintf("%38s",""),0,10,CT_BACK);
}
