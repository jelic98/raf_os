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

#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include "mode.h"

static unsigned int line_w = COLUMNS * 2;
static unsigned int square_w = 21;
static unsigned int square_h = 12;
static unsigned int curr_row = 0;
static unsigned int path_count = 0;
static unsigned int path_level = 0;

static char bl_gr = 0x0A;
static char bl_cy = 0x0B;
static char bl_re = 0x0C;
static char bl_pi = 0x0D;
static char bl_ye = 0x0E;
static char bl_wh = 0x0F;
static char wh_bl = 0xF0;

static char border = '#';
static char blank = ' ';
static char br_open = '[';
static char br_close = ']';
static char back_sp = 8;

static char parent_paths[20][20] = {"/"};
static char header[2][20] = {"/", "clipboard"};
static char content[2][10][20];

#define PATHS 0
#define CLIPBOARD 1

char color_path(const char* path, const char* name);
void abs_path(const char* path, const char* name, char* abs);

void c_mode() {
	if(!mode.o) {
		mode.o = 1;
		return;
	}

	curr_row = 0;
	
	mode.c = 1 - mode.c;

	mode.e = 0;
	mode.x = 0;
}

void a_mode_on() {
	mode.a = 1;
}

void a_mode_off() {
	mode.a = 0;
}

void e_mode() {
	if(!mode.c) {
		return;
	}
	
	mode.e = 1 - mode.e;
}

void putc(char c) {
	__asm__ __volatile__ (
		"call put_queue;"
		:: "a" (c), "b" (0)
	);
}

void copy_row() {
	if(!mode.a || mode.e || mode.x) {
		putc(' ');
		return;
	}

	char s[256];
	
	if(mode.c) {
		strcpy(s, content[mode.c][curr_row]);
	}else {
		abs_path(header[PATHS], content[mode.c][curr_row], s);
	}

	char* ps = s;

	while(putc(*(ps++)), *ps);
	
	mode.e = 0;
	mode.d = 1;
}

void add_char(char c) {	
	char* s = content[CLIPBOARD][curr_row];
	int len = strlen(s);

	if(c == back_sp) {
		s[len - 1] = 0;
	}else if(c > 31 && c < 127) {
		if(len < square_w - 2) {
			s[len] = c;	
		}
	}

	draw_square();
}

int go_up() {
	if(!mode.a) {
		return 1;
	}
	
	int limit = square_h - 3;

	if(!mode.c) {
		limit = path_count - 2;
	}

	curr_row = (--curr_row == -1) ? limit : curr_row;

	return 0;
}

int go_down() {
	if(!mode.a) {
		return 1;
	}
	
	int limit = square_h - 3;

	if(!mode.c) {
		limit = path_count - 2;
	}

	curr_row = (++curr_row > limit) ? 0 : curr_row;
	
	return 0;
}

int go_left() {
	if(!mode.a) {
		return 1;
	}
	
	if(mode.c || !path_level) {
		return 0;
	}

	strcpy(header[PATHS], parent_paths[--path_level]);
	mode.x = 0;
	
	return 0;
}

int go_right() {
	if(!mode.a) {
		return 1;
	}

	if(mode.c || mode.x) {
		return 0;
	}

	strcpy(parent_paths[path_level++], header[PATHS]);

	if(color_path(header[PATHS], content[PATHS][curr_row]) != bl_cy) {
		strcpy(header[PATHS], "ERROR");
		mode.x = 1;
	}else {
		if(header[PATHS][strlen(header[PATHS]) - 1] != '/') {
			strcat(header[PATHS], "/");
		}
	
		strcat(header[PATHS], content[PATHS][curr_row]);
		mode.x = 0;
	}
	
	curr_row = 0;

	return 0;
}

void abs_path(const char* path, const char* name, char* abs) {
	strcpy(abs, path);
	
	if(path[strlen(path) - 1] != '/') {
		abs[strlen(abs)] = '/';
	}
	
	strcat(abs, name);
}

char color_path(const char* path, const char* name) {
	char abs[256] = {0};
	abs_path(path, name, abs);
	
	struct m_inode* root_inode = iget(0x301, 1);
	current->root = root_inode;
	current->pwd = root_inode;
	
	struct stat d_stat;
	sys_stat(abs, &d_stat);
	
	iput(root_inode);
	current->root = NULL;
	current->pwd = NULL;
	
	if((d_stat.st_mode & S_IFMT) == S_IFDIR) {
		return bl_cy;
	}else if((d_stat.st_mode & S_IFMT) == S_IFCHR) {
		return bl_ye;
	}else if(d_stat.st_mode & S_IXUSR) {
		return bl_gr;
	}else {
		return bl_wh;
	}
}

void list_path(const char* path) {
	int i, j;

	for(i = 0; i < square_h - 2; i++) {
	 	for(j = 0; j < square_w - 2; j++) {
			content[PATHS][i][j] = 0;
		}
	}
	
	struct m_inode* root_inode = iget(0x301, 1);
	current->root = root_inode;
	current->pwd = root_inode;

	struct dirent entry;
	int curr_dir = open(path, O_RDONLY);

	path_count = 1;

	while(path_count) {
		int len = getdents(curr_dir, &entry, 1);

		if(len <= 0 || path_count > square_h - 2) {
			break;
		}

		char* name = entry.d_name;
		
		if(!strcmp(*name, '.')) {
			continue;
		}

		strcpy(content[PATHS][path_count++ - 1], name);
	}

	close(curr_dir);	

	iput(root_inode);
	current->root = NULL;
	current->pwd = NULL;
}

void draw_square() {
	if(!mode.o) {
		return;
	}

	if(!mode.c) {
		list_path(header[PATHS]);
	}

	int i, j;

	for(i = 0; i < square_h; i++) {
		for(j = 0; j < square_w; j++) {
			int pos = origin + (i + 1) * line_w - 2 * square_w + 2 * j;

			char c = blank;
			char color = bl_wh;
			
			if(i == 0) {
				int len = strlen(header[mode.c]);
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
					color = mode.a ? bl_gr : mode.e ? bl_pi : mode.x ? bl_re : bl_wh;
				}
			}else if(i == square_h - 1 || j == 0 || j == square_w - 1) {	
				c = border;
				color = mode.a ? bl_gr : mode.e ? bl_pi : mode.x ? bl_re : bl_wh;
			}else {	
				char* row = content[mode.c][i - 1];

				if(!mode.c) {
					color = color_path(header[PATHS], row);
				}

				int len = strlen(row);
				int l_bound = 0.5 * (square_w - len);
				int r_bound = 0.5 * (square_w + len);

				if(j >= l_bound && j < r_bound) {
					c = row[j - l_bound];	
				}

				if(i == curr_row + 1) {
					color = wh_bl;
				}
			}
			
			*(short*) pos = ((char) color << 8) | c;
		}
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
				} else if (c==27)
					state=1;
				else if (c==10 || c==11 || c==12)
					lf();
				else if (c==13)
					cr();
				else if (c==ERASE_CHAR(tty))
					del();
				else if (c==8) {
					if (x) {
						x--;
						pos -= 2;
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
