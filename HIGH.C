/*
 * high.c
 * Maintain highscore list
 */

#include "io.h"
#include "conio.h"
#include "dos.h"
#include "fcntl.h"
#include "sys\\stat.h"
#include "high.h"
#include "stdlib.h"
#include "string.h"
#include "game.h"
#include "oprintf.h"

static char high_loaded;
struct HIGH_SCORE hs[NUM_HIGHS];

/*
 * Write the high scores out to the data file
 */
static void write_high(void)
{
	int fd;

	fd=open(HIGH_FN,O_RDWR|O_BINARY|O_CREAT,S_IWRITE|S_IREAD);
	write(fd,&hs[0],sizeof(hs));
	close(fd);
}

/*
 * Load and/or regenerate high score list
 */
static void load_high(void)
{
	int fd,i,c,j;
	long s;

	if (high_loaded) return;
	fd=open(HIGH_FN,O_RDONLY|O_BINARY);
	if (fd>0) {
		read(fd,&hs[0],sizeof(hs));
		high_loaded=1;
		close(fd);
		return;
	}
	high_loaded=1;
	s=8150L;
	for (i=0;i<NUM_HIGHS;i++) {
		s-=(rand()&31)*10;
		hs[i].score=s;
		for (j=0;j<3;j++) {
			c=(rand()%26)+'A';
			hs[i].inits[j]=c;
		}
	}
	write_high();
}

/*
 * Display the high scores
 */
void disp_high(void)
{
	int i,sy;

	sy=HIGH_LINE;
	load_high();
	for (i=0;i<NUM_HIGHS;i++)
		scrmsg(oprintf("%7ld %3.3s",hs[i].score,hs[i].inits),13,sy++,CT_EMISS);
}

/*
 * Take the high score info off the screen
 */
void clear_high(void)
{
	int i;

	for (i=HIGH_LINE;i<NUM_HIGHS+HIGH_LINE;i++) {
		scrmsg(oprintf("%20s",""),13,i,CT_BACK);
	}
}

/*
 * Check and update the high score list
 */
void check_high(long score)
{
	int i,c,j;

	if (score<hs[NUM_HIGHS-1].score) return; /* Not on the list */
	for (i=0;i<NUM_HIGHS;i++)
		if (score>=hs[i].score) break;

	if (i<NUM_HIGHS-1)
		memmove(&hs[i+1],&hs[i],sizeof(hs[0])*(NUM_HIGHS-i-1));
	hs[i].score=score;
	memset(hs[i].inits,' ',sizeof(hs[i].inits));
	disp_high();
	scrmsg("PLEASE ENTER YOUR INITIALS",8,15,CT_FMISS);
	for (j=0;j<3;j++) {
		scrmsg(" ",21+j,HIGH_LINE+i,CT_FMISS);
		c=getch();
		if (c<' ' || c>126) {
			if (c==8 && j) --j;
			--j;
			continue;
		}
		hs[i].inits[j]=c;
		scrmsg(oprintf("%c",c),21+j,HIGH_LINE+i,CT_FMISS);
	}
	write_high();
	scrmsg(oprintf("%39s",""),0,15,CT_BACK);
	clear_high();
}
