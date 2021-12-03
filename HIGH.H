/*
 * high.h
 * highscore list struture
 */

#define NUM_HIGHS 6
#define HIGH_FN "HIGHSCOR"
#define HIGH_LINE 3

struct HIGH_SCORE {
	long score;
	char inits[3];
};

extern struct HIGH_SCORE hs[NUM_HIGHS];

void disp_high(void);
void check_high(long score);
void clear_high(void);
