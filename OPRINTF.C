/*
 * oprintf.c Print to a static output buffer like sprintf()
 * Created December 13, 1990, by Jeff Kesner
 *
 * oprintf works like sprintf, only it returns the address of its internal
 * string buffer.  It performs most of the interesting integer formats and
 * contains a "$" format which formats longs to dollars and cents
 * It is interesting to note that there are some minor differences between
 * TURBO C and Microsoft C.
 *
 */

#include "string.h"
#include "stdlib.h"
#include "stdarg.h"
#include "oprintf.h"

#define TURBOC

#ifdef TEST_SCRIPT
/*
 * The following is a test script for oprintf:
 *   cl /DTEST_SCRIPT=1 /Zp /W3 /Ox oprintf.c
 */

#include "stdio.h"
char *voprintf(char *fs, va_list arg_ptr);
char *oprintf(char *fs, ...);

void debug(char *fs, ...)
{
    static va_list arg_ptr;
    va_start(arg_ptr,fs);
    puts(fs);
    puts(voprintf(fs, arg_ptr));
    vprintf(fs,arg_ptr);
    puts("\n");
}

void main(void)
{
    int i=5;
    long l=1234567890L;
    debug("Num: %d",i);
    debug("Num1: %ld",(long)2000000000L);
	debug("!%-*.*s!",6,6,"abcdefg");
    debug("!%6s!","a");
	debug("!%6.6s!","abcdefg");
    debug("!%6.5s!","abcdefg");
    debug("!%-6.5s!","abcdefg");
    debug("!%+6.5s!","abcdefg");
	debug("%x",0x1a4c);
    debug("%%%X%%",0x1a4c);
    debug("!%4.4d!",2);
    debug("!%4.4d!",12345);
    debug("!%-4.3d!",2);
    debug("!%+4.3d!",2);
    debug("!%+4.3d!",-2);
    debug("!%4.3d!",2);
	debug("!%4d!",-2);
    debug("!%4d!",2);
    debug("%ld%ld%ld%ld%ld%ld%ld%ld%ld%ld%ld%ld%ld",l,l,l,l,l,l,l,l,l,l,l,l,l);
    debug("!%4s!","a");
    debug("!%-4s!","a");
    debug("!%+4s!","a");
    debug("!%-#4x!",0x4a7);
    debug("!%-#6.4x!",0x4a7);
    debug("!%+#6.4x!",0x4a7);
    debug("!%+#6.4!",0x4a7);
    debug("!%+#6.4x!",0xa);
    debug("!%+#6.4x!",-1);
    debug("!%+#6.4X!",-1);
    debug("!%+#6.4o!",0x4a7);
    debug("!%+#6.4o!",7);
    debug("!a=%4c!",'a');
    debug("$%5$$",396L);
    debug("$%5$$",0L);
    debug("$%5$$",l);
}
#endif

struct {
	char s[128];    /* Area for string being built */
	char *sp;       /* Pointer into string */
	int len;        /* String length */
	char work[34];  /* Work area */
} str;

/*
 * Make assignments to a string without overflowing boundaries
 */
static void sassign(char *s,int len)
{
	while (*s && len && str.len<sizeof(str.s)-1) {
        if (len!=-1) --len;
        *str.sp++=*s++;
        ++str.len;
    }
    *str.sp='\0';
}

char *voprintf(char *fs, va_list arg_ptr)
{
    str.sp=str.s;
    str.len=0;
    while (*fs) {
        switch (*fs++) {
        case '%': /* % escape */
            {
                struct {
                    char *backup;
                    int left;
                    int right;
                    unsigned have_left:1;
                    unsigned number:1;
                    unsigned blank:1;
                    unsigned plus:1;
                    unsigned minus:1;
                    unsigned done:1;
					unsigned longf:1;
                } state;
                state.backup=fs;
                state.left=state.right=-1;
                state.have_left=state.number=state.blank=state.plus=state.minus=state.done=state.longf=0;
                while (!state.done) {
                    char c;
                    switch (c=*fs++) {
                    case '\0': /* End of string before we expected it */
                        --fs;
                        state.done=1;
                        break;
                    case '%': /* %% */
                        state.done=1;
                        sassign("%",1);
                        break;
                    case ' ':
                        state.blank=1;
                        break;
                    case '#':
                        state.number=1;
                        break;
                    case '-':
                        state.minus=1;
                        break;
                    case '+': /* Justification flags */
                        state.plus=1;
                        break;
                    case '*': /* load parameter */
                        if (state.have_left) state.right=va_arg(arg_ptr,int);
                        else state.left=va_arg(arg_ptr,int);
                        break;
                    case '.': /* "precision follows" flag */
                        if (state.have_left) {
							state.done=1;
                            break;
                        }
                        state.have_left=1;
                        break;
                    case 'l': /* Long modifier */
                        state.longf=1;
                        break;
                    case 's': /* string */
                    case 'c': /* Character */
                    case '$': /* Dollar from long */
                        {
                            static int len,goal;
                            static char *s;

                            switch (c) {
                            case 'c':
                                {
                                    static char t[2];
                                    t[0]=(char)(va_arg(arg_ptr,int));
                                    t[1]='\0';
                                    s=t;
                                }
                                break;
                            case 's':
                                s=va_arg(arg_ptr,char *);
                                break;
                            case '$':
                                {
                                    static char work[15];
                                    static int len;
                                    static long cents;
                                    cents=va_arg(arg_ptr,long);
                                    ltoa(cents/100L,work,10);
									len=strlen(work);
                                    if (cents<0L) cents=-cents;
                                    work[len+3]=0;
                                    work[len+2]=(char)(cents%10L)+'0';
                                    cents/=10L;
                                    work[len+1]=(char)(cents%10L)+'0';
                                    work[len]='.';
                                    s=work;
                                }
                                break;
                            }
                            if (state.right!=-1) {
                                static char *t;
                                for (len=0,t=s;*t++ && len<state.right;len++);
                            } else len=strlen(s);
                            goal=max(state.left,len);
                            if (!state.minus && state.left!=-1)
                                while (len++<goal) sassign(" ",1);
                            sassign(s,state.right);
                            if (state.minus && state.left!=-1)
                                while (len++<goal) sassign(" ",1);
                            state.done=1;
                        }
                        break;
                    case 'X': /* Upper case hex */
                    case 'x': /* Lower case hex */
                    case 'd': /* integer */
                    case 'o': /* Octal */
                        {
                            static int vgoal,vlen,len, goal,radix;

                            switch (c) {
                            case 'X':
                            case 'x':
								radix=16;
                                break;
                            case 'o':
                                radix=8;
                                break;
                            default:
                                radix=10;
                                break;
                            }
                            if (state.longf) ltoa(va_arg(arg_ptr,long),str.work,radix);
                            else itoa(va_arg(arg_ptr,int),str.work,radix);
                            vlen=len=strlen(str.work);
                            if (state.plus && *str.work!='-' && *str.work!='0' && radix==10) {
                                memmove(str.work+1,str.work,vlen+1);
#ifdef TURBOC
                                ++vlen;
                                len++;
#else
                                if (state.left!=-1) --state.left;
#endif
                                *str.work='+';
                            }
                            if (*str.work=='-') ++state.right;
#ifdef TURBOC
                            if (radix==16 && state.number) vlen=len+=2;
                            if (radix==8 && state.number && *str.work!='0') ++len,++vlen;
#endif
                            vgoal=goal=max(state.right,len);
                            goal=max(goal,state.left);
                            if (!state.minus && state.left!=-1) {
                                while (vgoal++<goal) {
                                    ++len;
                                    sassign(" ",1);
                                }
							}
                            if (radix==16 && state.number) {
                                sassign((c=='x')?"0x":"0X",2);
                            }
                            if (radix==8 && state.number && *str.work!='0') {
                                sassign("0",1);
                            }
                            if (state.right!=-1) {
                                if (*str.work=='-' || *str.work=='+' ) {
                                    sassign(str.work,1);
                                    strcpy(str.work,str.work+1);
                                    len++;
                                }
                                while (vlen++<state.right) {
                                    ++len;
                                    sassign("0",1);
                                }
                            }
                            if (c=='X') strupr(str.work);
                            sassign(str.work,-1);
                            if (state.minus && state.left!=-1)
                                while (len++<goal) sassign(" ",1);
                            state.done=1;
                        }
                        break;
                    default: /* Possibly numeric */
                        if (c>='0' && c<='9') {
                            if (state.have_left) state.right=atoi(fs-1);
                            else state.left=atoi(fs-1);
                            while (*fs>='0' && *fs<='9') ++fs;
                        } else {
                            state.done=1;       /* Error condition */
#ifdef TURBOC
                            sassign("%",1);
                            fs=state.backup;
#else
                            sassign(fs-1,1);
#endif
                        }
                    }
                }
                break;
            }
        default: /* Wasn't a '%' escape */
            sassign(fs-1,1);
        }
    }
    *str.sp='\0';
    return(str.s);
}

/*
 * oprintf: sprintf to an internal buffer and return a pointer to the buffer
 */
char *oprintf(char *fs, ...)
{
    static va_list arg_ptr;
    va_start(arg_ptr,fs);
    return(voprintf(fs, arg_ptr));
}
