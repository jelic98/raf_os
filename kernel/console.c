/*
 *	console.c
 *
 * This module implements the console io functions
 *	'void con_init(void)'
 *	'void con_write(struct tty_queue * queue)'
 * Hopefully this will be a rather complete VT102 implementation.
 *
 */

/*
 *  NOTE!!! We sometimes disable and enable interrupts for a short while
 * (to put a word in video IO), but this will work even for keyboard
 * interrupts. We know interrupts aren't enabled when getting a keyboard
 * interrupt, as we use trap-gates. Hopefully all is well.
 */

#include <linux/sched.h>
#include <linux/tty.h>
#include <asm/io.h>
#include <asm/system.h>

#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>

#define SCREEN_START 0xb8000
#define SCREEN_END   0xc0000
#define LINES 25
#define COLUMNS 80
#define NPAR 16

extern void keyboard_interrupt(void);

static unsigned long origin=SCREEN_START;
static unsigned long scr_end=SCREEN_START+LINES*COLUMNS*2;
static unsigned long pos;
static unsigned long x,y;
static unsigned long top=0,bottom=LINES;
static unsigned long lines=LINES,columns=COLUMNS;
static unsigned long state=0;
static unsigned long npar,par[NPAR];
static unsigned long ques=0;
static unsigned char attr=0x07;

/*
 * this is what the terminal answers to a ESC-Z or csi0c
 * query (= vt100 response).
 */
#define RESPONSE "\033[?1;2c"

// DOMACI

static volatile struct {
	unsigned int o : 1; // 0 = closed, 1 = open
	unsigned int c : 1; // 0 = explorer, 1 = clipboard
	unsigned int a : 1; // 0 = f1 released, 1 = f1 pressed
	unsigned int e : 1; // 0 = input disabled, 1 = input enabled
} mode;

static unsigned int pos_start = SCREEN_START;
static unsigned int line_w = COLUMNS * 2;
static unsigned int square_w = 21;
static unsigned int square_h = 12;
static unsigned int curr_row = 0;
static volatile char wh_bl = 0xF0;
static volatile char bl_wh = 0x0F;
static volatile char bl_gr = 0x0A;
static char border = '#';
static char blank = ' ';
static char br_open = '[';
static char br_close = ']';
static char back_sp = 8;
static char header[2][20] = {"/", "clipboard"};
static char paths[10][20] = {0};
static char clipboard[10][20] = {0};

void c_mode() {
	if(!mode.o) {
		mode.o = 1;
		return;
	}
	
	mode.c = 1 - mode.c;
}

void a_mode_on() {
	mode.a = 1;
}

void a_mode_off() {
	mode.a = 0;
}

void e_mode() {
	mode.e = 1 - mode.e;
}

void copy_row() {
	if(!mode.a) {
		return;
	}

	mode.e = 0;
}

void go_up() {
	if(!mode.a) {
		return;
	}
	
	curr_row = (--curr_row == -1) ? square_h - 3 : curr_row;
}

void go_down() {
	if(!mode.a) {
		return;
	}

	curr_row = (++curr_row > square_h - 3) ? 0 : curr_row;
}

void go_left() {

}

void go_right() {

}

void add_char(char c) {
	char* s = clipboard[curr_row];
	int len = strlen(s);

	if(c > 31 && c < 127) {
		if(len < square_w - 2) {
			s[len] = c;
		}
	}else {
		s[len - 1] = 0;
	}

	draw_square();
}

void draw_square() {
	if(!mode.o) {
		return;
	}
	
	int i, j;

	for(i = 0; i < square_h; i++) {
		for(j = 0; j < square_w; j++) {
			int pos = pos_start + (i + 1) * line_w - 2 * square_w + 2 * j;

			char c = blank;
			char color = bl_wh;
			
			if(i == 0) {
				int len = strlen(header[mode.c]);
				//int len = list_path(".");
				int l_bound = 0.5 * (square_w - len) - 1;
				int r_bound = 0.5 * (square_w + len);

				if(j == l_bound - 1) {
					c = br_open;
				}else if(j == r_bound + 1) {
					c = br_close;
				}else if(j == l_bound || j == r_bound) {
					c = blank;
				}else if(j > l_bound && j < r_bound) {
					c = header[mode.c][j - l_bound - 1];
				}else {
					c = border;
				}
			}else if(i == square_h - 1 || j == 0 || j == square_w - 1) {	
				c = border;
			}else {
				if(i == curr_row + 1) {
					color = wh_bl;
				}
				
				char* row = paths[i - 1];

				if(mode.c) {
					row = clipboard[i - 1];
				}

				int len = strlen(row);
				int l_bound = 0.5 * (square_w - len);
				int r_bound = 0.5 * (square_w + len);

				if(j >= l_bound && j < r_bound) {
					c = row[j - l_bound];
				}
			}

			__asm__ __volatile__(
				"movw %%ax, (%%ebx);"
				:: "a" ((color << 8) | c), "b" (pos)
			);
		}
	}
}

int list_path(char* path) {
	struct dirent entry;
	int count = 0;

	int curr_fd = open(path, O_RDONLY);

	while(++count) {
		int len = getdents(curr_fd, &entry, 1);

		if(len > 0) {
			write(1, entry.d_name, len);
			write(1, "\n", 1);
		}else {
			break;
		}
	}

	close(curr_fd);

	return count - 1;
}

void dump(char* s) {
	int i = -1;

	while(i++, *s) {
		__asm__ __volatile__(
			"movb bl_gr, %%ah;"
			"movw %%ax, (%%ebx);"
			:: "a" (*(s++)), "b" (pos_start + 2 * i)
		);
	}
}

static inline void gotoxy(unsigned int new_x,unsigned int new_y)
{
	if (new_x>=columns || new_y>=lines)
		return;
	x=new_x;
	y=new_y;
	pos=origin+((y*columns+x)<<1);
}

static inline void set_origin(void)
{
	cli();
	outb_p(12,0x3d4);
	outb_p(0xff&((origin-SCREEN_START)>>9),0x3d5);
	outb_p(13,0x3d4);
	outb_p(0xff&((origin-SCREEN_START)>>1),0x3d5);
	sti();
}

static void scrup(void)
{
	if (!top && bottom==lines) {
		origin += columns<<1;
		pos += columns<<1;
		scr_end += columns<<1;
		if (scr_end>SCREEN_END) {
			
			int d0,d1,d2,d3;
			__asm__ __volatile("cld\n\t"
				"rep\n\t"
				"movsl\n\t"
				"movl %[columns],%1\n\t"
				"rep\n\t"
				"stosw"
				:"=&a" (d0), "=&c" (d1), "=&D" (d2), "=&S" (d3)
				:"0" (0x0720),
				 "1" ((lines-1)*columns>>1),
				 "2" (SCREEN_START),
				 "3" (origin),
				 [columns] "r" (columns)
				:"memory");

			scr_end -= origin-SCREEN_START;
			pos -= origin-SCREEN_START;
			origin = SCREEN_START;
		} else {
			int d0,d1,d2;
			__asm__ __volatile("cld\n\t"
				"rep\n\t"
				"stosl"
				:"=&a" (d0), "=&c" (d1), "=&D" (d2) 
				:"0" (0x07200720),
				"1" (columns>>1),
				"2" (scr_end-(columns<<1))
				:"memory");
		}
		set_origin();
	} else {
		int d0,d1,d2,d3;
		__asm__ __volatile__("cld\n\t"
			"rep\n\t"
			"movsl\n\t"
			"movl %[columns],%%ecx\n\t"
			"rep\n\t"
			"stosw"
			:"=&a" (d0), "=&c" (d1), "=&D" (d2), "=&S" (d3)
			:"0" (0x0720),
			"1" ((bottom-top-1)*columns>>1),
			"2" (origin+(columns<<1)*top),
			"3" (origin+(columns<<1)*(top+1)),
			[columns] "r" (columns)
			:"memory");
	}
}

static void scrdown(void)
{
	int d0,d1,d2,d3;
	__asm__ __volatile__("std\n\t"
		"rep\n\t"
		"movsl\n\t"
		"addl $2,%%edi\n\t"	/* %edi has been decremented by 4 */
		"movl %[columns],%%ecx\n\t"
		"rep\n\t"
		"stosw"
		:"=&a" (d0), "=&c" (d1), "=&D" (d2), "=&S" (d3)
		:"0" (0x0720),
		"1" ((bottom-top-1)*columns>>1),
		"2" (origin+(columns<<1)*bottom-4),
		"3" (origin+(columns<<1)*(bottom-1)-4),
		[columns] "r" (columns)
		:"memory");
}

static void lf(void)
{
	if (y+1<bottom) {
		y++;
		pos += columns<<1;
		return;
	}
	scrup();
}

static void ri(void)
{
	if (y>top) {
		y--;
		pos -= columns<<1;
		return;
	}
	scrdown();
}

static void cr(void)
{
	pos -= x<<1;
	x=0;
}

static void del(void)
{
	if (x) {
		pos -= 2;
		x--;
		*(unsigned short *)pos = 0x0720;
	}
}

static void csi_J(int par)
{
	long count;
	long start;

	switch (par) {
		case 0:	/* erase from cursor to end of display */
			count = (scr_end-pos)>>1;
			start = pos;
			break;
		case 1:	/* erase from start to cursor */
			count = (pos-origin)>>1;
			start = origin;
			break;
		case 2: /* erase whole display */
			count = columns*lines;
			start = origin;
			break;
		default:
			return;
	}
	int d0,d1,d2;
	__asm__ __volatile__("cld\n\t"
		"rep\n\t"
		"stosw\n\t"
		:"=&c" (d0), "=&D" (d1), "=&a" (d2)
		:"0" (count),"1" (start),"2" (0x0720)
		:"memory");
}

static void csi_K(int par)
{
	long count;
	long start;

	switch (par) {
		case 0:	/* erase from cursor to end of line */
			if (x>=columns)
				return;
			count = columns-x;
			start = pos;
			break;
		case 1:	/* erase from start of line to cursor */
			start = pos - (x<<1);
			count = (x<columns)?x:columns;
			break;
		case 2: /* erase whole line */
			start = pos - (x<<1);
			count = columns;
			break;
		default:
			return;
	}
	int d0,d1,d2;
	__asm__ __volatile__("cld\n\t"
		"rep\n\t"
		"stosw\n\t"
		:"=&c" (d0), "=&D" (d1), "=&a" (d2)
		:"0" (count),"1" (start),"2" (0x0720)
		:"memory");
}

void csi_m(void)
{
	int i;

	for (i=0;i<=npar;i++)
		switch (par[i]) {
			case 0:attr=0x07;break;
			case 1:attr=0x0f;break;
			case 4:attr=0x0f;break;
			case 7:attr=0x70;break;
			case 27:attr=0x07;break;
		}
}

static inline void set_cursor(void)
{
	cli();
	outb_p(14,0x3d4);
	outb_p(0xff&((pos-SCREEN_START)>>9),0x3d5);
	outb_p(15,0x3d4);
	outb_p(0xff&((pos-SCREEN_START)>>1),0x3d5);
	sti();
}

static void respond(struct tty_struct * tty)
{
	char * p = RESPONSE;

	cli();
	while (*p) {
		PUTCH(*p,tty->read_q);
		p++;
	}
	sti();
	copy_to_cooked(tty);
}

static void insert_char(void)
{
	int i=x;
	unsigned short tmp,old=0x0720;
	unsigned short * p = (unsigned short *) pos;

	while (i++<columns) {
		tmp=*p;
		*p=old;
		old=tmp;
		p++;
	}
}

static void insert_line(void)
{
	int oldtop,oldbottom;

	oldtop=top;
	oldbottom=bottom;
	top=y;
	bottom=lines;
	scrdown();
	top=oldtop;
	bottom=oldbottom;
}

static void delete_char(void)
{
	int i;
	unsigned short * p = (unsigned short *) pos;

	if (x>=columns)
		return;
	i = x;
	while (++i < columns) {
		*p = *(p+1);
		p++;
	}
	*p=0x0720;
}

static void delete_line(void)
{
	int oldtop,oldbottom;

	oldtop=top;
	oldbottom=bottom;
	top=y;
	bottom=lines;
	scrup();
	top=oldtop;
	bottom=oldbottom;
}

static void csi_at(int nr)
{
	if (nr>columns)
		nr=columns;
	else if (!nr)
		nr=1;
	while (nr--)
		insert_char();
}

static void csi_L(int nr)
{
	if (nr>lines)
		nr=lines;
	else if (!nr)
		nr=1;
	while (nr--)
		insert_line();
}

static void csi_P(int nr)
{
	if (nr>columns)
		nr=columns;
	else if (!nr)
		nr=1;
	while (nr--)
		delete_char();
}

static void csi_M(int nr)
{
	if (nr>lines)
		nr=lines;
	else if (!nr)
		nr=1;
	while (nr--)
		delete_line();
}

static int saved_x=0;
static int saved_y=0;

static void save_cur(void)
{
	saved_x=x;
	saved_y=y;
}

static void restore_cur(void)
{
	x=saved_x;
	y=saved_y;
	pos=origin+((y*columns+x)<<1);
}

void con_write(struct tty_struct * tty)
{
	int nr;
	char c;

	nr = CHARS(tty->write_q);
	while (nr--) {
		GETCH(tty->write_q,c);
		switch(state) {
			case 0:
				if (c>31 && c<127) {
					// DOMACI
					if(mode.e) {
						add_char(c);
					}else {
						if (x>=columns) {
							x -= columns;
							pos -= columns<<1;
							lf();
						}
						__asm__("movb attr,%%ah\n\t"
							"movw %%ax,%1\n\t"
							::"a" (c),"m" (*(short *)pos)
							/*:"ax"*/);
						pos += 2;
						x++;
					}	
				} else if (c==27)
					state=1;
				else if (c==10 || c==11 || c==12)
					lf();
				else if (c==13)
					cr();
				else if (c==ERASE_CHAR(tty))
					del();
				else if (c==8) {
					// DOMACI
					if(mode.e) {
						add_char(c);
					}else {
						if (x) {
							x--;
							pos -= 2;
						}
					}
				} else if (c==9) {
					c=8-(x&7);
					x += c;
					pos += c<<1;
					if (x>columns) {
						x -= columns;
						pos -= columns<<1;
						lf();
					}
					c=9;
				}
				break;
			case 1:
				state=0;
				if (c=='[')
					state=2;
				else if (c=='E')
					gotoxy(0,y+1);
				else if (c=='M')
					ri();
				else if (c=='D')
					lf();
				else if (c=='Z')
					respond(tty);
				else if (x=='7')
					save_cur();
				else if (x=='8')
					restore_cur();
				break;
			case 2:
				for(npar=0;npar<NPAR;npar++)
					par[npar]=0;
				npar=0;
				state=3;
				if ((ques=(c=='?')))
					break;
			case 3:
				if (c==';' && npar<NPAR-1) {
					npar++;
					break;
				} else if (c>='0' && c<='9') {
					par[npar]=10*par[npar]+c-'0';
					break;
				} else state=4;
			case 4:
				state=0;
				switch(c) {
					case 'G': case '`':
						if (par[0]) par[0]--;
						gotoxy(par[0],y);
						break;
					case 'A':
						if (!par[0]) par[0]++;
						gotoxy(x,y-par[0]);
						break;
					case 'B': case 'e':
						if (!par[0]) par[0]++;
						gotoxy(x,y+par[0]);
						break;
					case 'C': case 'a':
						if (!par[0]) par[0]++;
						gotoxy(x+par[0],y);
						break;
					case 'D':
						if (!par[0]) par[0]++;
						gotoxy(x-par[0],y);
						break;
					case 'E':
						if (!par[0]) par[0]++;
						gotoxy(0,y+par[0]);
						break;
					case 'F':
						if (!par[0]) par[0]++;
						gotoxy(0,y-par[0]);
						break;
					case 'd':
						if (par[0]) par[0]--;
						gotoxy(x,par[0]);
						break;
					case 'H': case 'f':
						if (par[0]) par[0]--;
						if (par[1]) par[1]--;
						gotoxy(par[1],par[0]);
						break;
					case 'J':
						csi_J(par[0]);
						break;
					case 'K':
						csi_K(par[0]);
						break;
					case 'L':
						csi_L(par[0]);
						break;
					case 'M':
						csi_M(par[0]);
						break;
					case 'P':
						csi_P(par[0]);
						break;
					case '@':
						csi_at(par[0]);
						break;
					case 'm':
						csi_m();
						break;
					case 'r':
						if (par[0]) par[0]--;
						if (!par[1]) par[1]=lines;
						if (par[0] < par[1] &&
						    par[1] <= lines) {
							top=par[0];
							bottom=par[1];
						}
						break;
					case 's':
						save_cur();
						break;
					case 'u':
						restore_cur();
						break;
				}
		}
	}
	set_cursor();
}

/*
 *  void con_init(void);
 *
 * This routine initalizes console interrupts, and does nothing
 * else. If you want the screen to clear, call tty_write with
 * the appropriate escape-sequece.
 */
void con_init(void)
{
	register unsigned char a;

	gotoxy(*(unsigned char *)(0x90000+510),*(unsigned char *)(0x90000+511));
	set_trap_gate(0x21,&keyboard_interrupt);
	outb_p(inb_p(0x21)&0xfd,0x21);
	a=inb_p(0x61);
	outb_p(a|0x80,0x61);
	outb(a,0x61);
}
