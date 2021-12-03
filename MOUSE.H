#define M_RESET			0
#define M_SHOW_CURS		1
#define M_HIDE_CURS		2
#define M_GET_STATUS    3
#define M_SET_CURS  	4
#define M_GET_PRESS 	5
#define M_GET_REL   	6
#define M_SET_HOR_LIM   7
#define M_SET_VER_LIM   8
#define M_SET_G_CURS    9
#define M_SET_T_CURS    10
#define M_READ_COUNT    11
#define M_USER_SUB  	12
#define M_LPEN_ON   	13
#define M_LPEN_OFF  	14
#define M_MICK_2_PIX    15
#define M_COND_OFF  	16
#define M_D_SPEED   	19

struct MOUSE_STAT {
	int x;
	int y;
	union {
		struct {
			unsigned l:1;
			unsigned r:1;
			unsigned m:1;
		} bit;
		int byte;
	} but;
};

void mouse(int *rax, int *rbx, int *rcx, int *rdx);
int mouse_reset(void);
void mouse_stat(struct MOUSE_STAT *p);
void mouse_limit(int x1,int x2,int y1,int y2);
void mouse_limits(int x1,int x2,int y1,int y2);
