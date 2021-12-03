/*
 * pixel.h
 * Routines contained in pixel.asm
 */

extern int pixel(int x, int y, int val);
extern char *dimage(char *s, char *end);
extern int	init_mode(int mode);
extern void	init_pal(char *pal,int first,int last);
extern void char_cor(int col);
extern int scankey(void);
extern int waitkey(void);

struct PAL {
	unsigned char red;
	unsigned char green;
	unsigned char blue;
};
