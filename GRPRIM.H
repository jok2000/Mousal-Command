/*
 * grprim.h
 * Graphics primitive definitions
 */

#define X_SIZE 320
#define Y_SIZE 200
#define SCR_SEG 0xa000

struct GR_COORD {
	int x;
	int y;
};

struct GR_LINE {	/* draw line status control block */
	struct GR_COORD src;
	struct GR_COORD dst;
	struct {
		int numer;
		int denom;
	} inc;
	struct GR_COORD cur;
	struct GR_COORD dir;
	int cur_frac;
	int xory;
	int cnt;
	int color;
	struct {
		int cnt;
		int max;
	} phase;
	int kind;		/* Line style */
					/* For LN_DEMO, includes base<<3 and cross_ref<<5 */
	int target;		/* Intended target */
	int demo;		/* Slated for demolition flag */
	int demo_time;	/* Expected time of intersect */
	int demo_list;	/* Corresponding request for demolition */
};

struct GR_OBJECT {
	int width;
	int height;
	char *bitmap;
	char *back;
	int x;
	int y;
};

extern char pal[256][3];

void inc_line(struct GR_LINE *line);
void init_line(struct GR_LINE *line);
void draw_line(int sx, int sy, int dx, int dy, int color);
int  sprite(struct GR_OBJECT *g,int state);
void draw_circle(int x,int y,int r,int c);
void pixc(int x,int y,int c);
void scroll_info(int ln);
void clear_line(struct GR_LINE *line);
void pal_over(int color);
void scrmsg(char *str,int x,int y,int color);
void scr_clear(void);
