/*
 * game.h - Jan. 26, 1992
 * Game-specific information
 */

#include "grprim.h"
#include "mouse.h"
#include "freelist.h"

struct GAME {
	long bonus;		/* Score multiple for bonus */
	int cities;		/* Number of cities given at start */
	char players;	/* Number of players */
	int start_mode;	/* Screen mode on entry to program */
	int active;		/* Something happenning? */
	int	cur_player;	/* Current player */
	struct {
		unsigned sound:1;	/* Sound active? */
		unsigned mouse:1;
	} flags;
	volatile char sync;		/* Synchronization delay flag */
	int sync_cnt;	/* Synchronization counter */
	int	times;		/* Score multiplier */
	long prev_score;/* Score at start of set */
	unsigned char last_set; /* last set played */
	unsigned char demo_set;	/* Demo set level */
};

extern struct GAME game;

#define ACTIVITY 333	/* 1 second */
#define MAX_LINES 40

struct PLAYER {
	unsigned char set; 	/* Current set being played */
	long score;		/* Current score */
	char cities;    /* Number of cities banked */
	char city[6];	/* Cities occupied */
	char mbase[3];	/* Missiles in each base */
};

struct GR_CIRC {
	int x;
	int y;
	int phase;
	int dir;
	int stage;
	int kind;
};

enum LINE_STYLE { LN_EMISS, LN_FMISS, LN_SMART, LN_DEMO};

enum P_COLORS {
	C_BLACK, C_RED, C_GREEN, C_YELLOW, C_BLUE, C_MAGENTA, C_CYAN, C_WHITE
};

/*
 * Color types are as follows:
 * CT_BACK for background colors
 * CT_FMISS for friendly missle colors
 * CT_EMISS for enemy missle colors
 * CT_LAND for land colors
 * CT_HOT for white-hot but not explosive colors e.g., X
 * CT_EXPLO for explosively hot explosions
 * CT_CROSS for cross hair special color
 */
enum COLOR_TYPES {
	CT_BACK, CT_FMISS, CT_EMISS, CT_HOT, CT_EXPLO, CT_CROSS, CT_NUM
};

enum TARGETS {TR_NONE, TR_CITY=8, TR_BASE=16};
enum L_WHERE {L_TOP, L_SHIP, L_MIRV};
struct TARGET {
	struct {
		int cnt;		/* Number of attackers on this target */
		int tcnt;		/* Total launched on this target this set */
		int kind;		/* target type (TARGETS) */
	} targetted[9];		/* targeted objects */
	int taken;			/* Number of cities taken so far */
	int lcity;			/* Number of live cities targetted */
	int wave;			/* Main wave counter */
	int swave;			/* Sub-wave counter */
	int lphase;			/* Launch phase count */
	int llines;			/* Number of lines to launch */
	int lsmarts;		/* Number of smarts to launch */
	int stage;			/* Distance down screen of main wave */
	int where;			/* Where to launch from (L_WHERE) */
	int mx;
	int my;				/* MIRV coordinates */
	int noattack;		/* Cease hostilities */
	int fasttrack;		/* Speed up lines etc. */
};
struct TARGET_RET {		/* targetter return info */
	int index;			/* Target index */
	int x;
	int y;
};

struct SOUND_CB {
	char kind;
	int  c_freq;
	int	 s_freq;
	int  c_step;
	int  s_step;
	int  s_step_cnt;
	int  c_step_cnt;
	int  step2;
	int	 s_phase;
	int  c_phase;
	int	 repeat;
};

#define SAT_PHASE_MAX 10 /* Speed of satellite */
struct SAT {
	char *plot_dat;	/* Plotter data */
	int x;
	int y;
	int phase;		/* Wait state indicator */
	int dir;		/* Direction of motion */
	int demo;		/* Slated for demolition? */
};

struct SDAT {			/* Copy of "Enemy attack characteristics" */
	char silo_missiles;	/* How many he's got in silos */
	char ships;			/* Max. Bombers expected */
	char silo_smarts;	/* How man he's got in silos */
	char speed;			/* The speed of the enemy missiles in heart beats */
	char wave; 			/* Attack wave */
	int  wait;			/* Waiting period */
	char air_missiles;  /* Missiles in the air */
	char air_smarts;	/* smarts in the air */
	char air_abm;		/* ABM's in the air */
	char air_demo;		/* Demo target flag */
};

struct MLIMIT {
	struct LXY {
		int low;
		int high;
	} x;
	struct LXY y;
};
struct DTARG {        	/* Structure to handle demo-mode targetting */
	int phase;
	int count;
	int first;
	int last;
	long time;
	struct {
		int x;
		int y;
		int base;
		int ignore;
	} targs[MAX_LINES];
	int base[3];
	int cspeed;
};

extern struct DTARG dtarg;
extern char pal_color[8][3];
extern struct GR_COORD cities[6];
extern struct GR_COORD bases[3];
extern struct SDAT sdat;
extern struct GR_OBJECT cross_hair;
extern struct F_LIST flist_line[MAX_LINES+1];
extern struct MLIMIT mlimit;
extern struct GR_COORD cityb[7];
extern char sat_dat[];
extern struct SAT sat;
extern struct GR_LINE msl[MAX_LINES];
extern struct GR_OBJECT smart_bomb;
extern char bomber_dat[];
extern struct GR_OBJECT base_mis;
extern struct PLAYER player[3];
extern struct TARGET target;

int plot_sat(char *dat, int x, int y, int dir);
void new_circle(int x,int y,int kind);
void update_lines(void);
void the_end(void);
void intro(void);
void init(void);
void inc_score(int val);
void gdelay(int del);
void free_target(int index);
void kill_target(int index);
void draw_city(int c,int st,struct GR_COORD *cp);
void play(void);
void pal_cycle(void);
void pal_setup(int set);
void draw_set(int pl);
void init_sound(int kind);
void init_set(int p);
void demo_targ(void);
void init_player(int p);
void handle_dmis(int base);
void handle_mis(int base,struct MOUSE_STAT *m);
void update_bomber(void);
void offense(void);
void new_line(int sx, int sy, int dx, int dy, int phase,int kind,int target);
void sound_update(void);
void update_circles(void);
void sync(void);
