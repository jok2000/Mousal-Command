#include "mouse.h"
#include "dos.h"

/*
 * Perform a mouse interrupt function
 */
void mouse(int *m1, int *m2, int *m3, int *m4)
{
	union REGS rin, rout;

	rin.x.ax=*m1;
	rin.x.bx=*m2;
	rin.x.cx=*m3;
	rin.x.dx=*m4;
	int86(0x33, &rin, &rout);
	*m1=rout.x.ax;
	*m2=rout.x.bx;
	*m3=rout.x.cx;
	*m4=rout.x.dx;
}

/*
 * Initialize the mouse
 */
int mouse_reset(void)
{
	int m1,m2,m3,m4;

	m1 = M_RESET;
	mouse(&m1, &m2, &m3, &m4);
	return(m1);
}

/*
 * Set maximum area of mouse movements
 */
void mouse_limits(int x1,int x2,int y1,int y2)
{
	int m1,m2,m3,m4;

	m1 = M_SET_HOR_LIM;
	m2 = 0;
	m3 = x1;
	m4 = x2;
	mouse(&m1, &m2, &m3, &m4);

	m1 = M_SET_VER_LIM;
	m2 = 0;
	m3 = y1;
	m4 = y2;
	mouse(&m1, &m2, &m3, &m4);
}

/*
 * Get the button positions and mouse coordinates
 */
void mouse_stat(struct MOUSE_STAT *p)
{
	int m1,m2,m3,m4;

	m1 = M_GET_STATUS;
	mouse(&m1, &m2, &m3, &m4);
	p->x=m3;
	p->y=m4;
	p->but.byte=m2;
}