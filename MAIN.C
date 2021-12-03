/*
 * Test graphics
 */

#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "game.h"

/*
 * Read the command line arguments and do only the highest level control
 */
void main(int argc, char **argv)
{
	int a;

	/*
	 * Your basic arcade owner controls;
	 *   Bonus, starting compliment of cities, game speed, sound
	 */
	game.flags.mouse=game.flags.sound=1;
	for (a=1;a<argc;a++) {
		if (!stricmp(argv[a],"nosound")) game.flags.sound=0;
		else if (!stricmp(argv[a],"cities")) game.cities=atoi(argv[++a]);
		else if (!stricmp(argv[a],"bonus")) game.bonus=atol(argv[++a]);
		else if (!stricmp(argv[a],"sync")) game.sync_cnt=atoi(argv[++a]);
		else if (!stricmp(argv[a],"nomouse")) game.flags.mouse=0;
	}
	if (!game.bonus) game.bonus=8000L;
	if (!game.cities) game.cities=6;
	if (game.sync_cnt<1000) game.sync_cnt=3585;

	/*
	 * Let the festivities begin!
	 */
	init();
	for (;;) {
		intro();
		play();
		the_end();
	}
}
