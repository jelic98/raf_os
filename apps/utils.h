#ifndef H_UTILS_INCLUDE
#define H_UTILS_INCLUDE

#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

#define STD_IN 0
#define STD_OUT 1
#define STD_ERROR 2

#define ARG_PAT -1
#define ARG_PWD -2
#define ARG_SHE -3
#define ARG_HOM -4
#define ARG_EXE -5

#define BLOCK_LENGTH 1024
#define FILE_LENGTH 4096
#define BUFFER_LENGTH 128
#define ASCII_OFFSET 48
#define STRING_END '\0'
#define NEW_LINE '\n'
#define NEW_LINE_STRING "\n"

#define MSG_FILE_NAME "File name: \0"
#define MSG_FILE_ERROR "Cannot open selected file\0"

#define __isdigit(x) ((x >= '0') && (x <= '9'))

#define vardump(x) {\
	char buf[BUFFER_LENGTH];\
	int len;\
	len = itoa(x, buf);\
	write(STD_OUT, #x ": ", strlen(#x ": "));\
	write(STD_OUT, buf, len);\
	write(STD_OUT, "\n", 1);\
}

static char* errtxt[] = {
	/* 0 */ 	"NULL\0","NULL\0","NULL\0","NULL\0","NULL\0",
	/* 5 */ 	"NULL\0","NULL\0","NULL\0","NULL\0","NULL\0",
	/* 10 */ 	"NULL\0","NULL\0","NULL\0","NULL\0","NULL\0",
	/* 15 */ 	"NULL\0","NULL\0","NULL\0","NULL\0","NULL\0",
	/* 20 */ 	"NULL\0","NULL\0","NULL\0","NULL\0","NULL\0",
	/* 25 */ 	"NULL\0","NULL\0","NULL\0","NULL\0","NULL\0",
	/* 30 */ 	"NULL\0","NULL\0","NULL\0","NULL\0","NULL\0",
	/* 35 */ 	"NULL\0","NULL\0","NULL\0","NULL\0","NULL\0",
	/* 40 */ 	"Key length is not the power ot two\0",
	/* 41 */	"Key level is not in range\0",
	/* 42 */	"File is already encrypted\0",
	/* 43 */	"File is already decrypted\0",
	/* 44 */	"Missing arguments\0",
	/* 44 */	"Required key not available\0"
};

int digitcount(int n);

int itoa(int n, char* buf);
int atoi(const char* buf);

void reverse(char* buf, int len);
char* newln(char* buf, int len);

void scan(char* buf);
int scannum();

int fopen(char* name);
void fclose(int fd);
int fgets(char* buf, int len, int fd);
int fputs(char* buf, int len, int fd);

void print(char* buf);
void println(char* buf);
void printnum(int n);

void pause(void);

int get_argc(char *args);
char *get_argv(char *args, int argnum);

void checkarg(char* args, int n);
void printerr(int code);
void checkerr();

#ifdef H_UTILS_IMPLEMENT

int digitcount(int n) {
	int count = -1;

	while(count++, n > 0) {
		n /= 10;
	}

	return count;
}

void reverse(char* buf, int len) {
	int i, j;
	char c;

	for(i = 0, j = len - 1; i < j; ++i, --j) {
		c = buf[i];
		buf[i] = buf[j];
		buf[j] = c;
	}
}

int itoa(int n, char* buf) {
	int i = 0, sgn;

	if((sgn = n) < 0) {
		n = -n;
	}

	do{
		buf[i++] = n % 10 + ASCII_OFFSET;
	}while((n /= 10) > 0);

	if(sgn < 0) {
		buf[i++] = '-';
	}

	buf[i] = '\0';
	
	reverse(buf, i);
	
	return i;
}

int atoi(const char* buf) {
	int r = 0, i;

	for(i = 0; __isdigit(buf[i]); ++i) {
		r = r * 10 + buf[i] - ASCII_OFFSET;
	}

	return r;
}

char* newln(char* buf, int len) {
	buf[len] = NEW_LINE;

	return buf;
}

void scan(char* buf) {
	buf[read(STD_IN, buf, BUFFER_LENGTH) - 1] = STRING_END;
}

int scannum() {
	char buf[BUFFER_LENGTH];

	scan(buf);

	return atoi(buf);
}

int fopen(char* name) {
	int fd = open(name, O_RDONLY);

	if(fd == -1) {
		print(MSG_FILE_ERROR);
		_exit(1);
	}

	return fd;
}

void fclose(int fd) {
	close(fd);
}

int fgets(char* buf, int len, int fd) {
	int i = 0;
	char c;

	do {
		if(!read(fd, &c, 1) || i == len - 1) {
			break;
		}

		buf[i++] = c;
	}while(c != '\n' && c!='\0');
	
	buf[i] = '\0';
	
	return i;
}

int fputs(char* buf, int len, int fd) {
	write(fd, buf, len);

	return len;
}

void print(char* buf) {
	int len = strlen(buf);

	buf[len] = STRING_END;
	
	write(STD_OUT, buf, len + 1);
}

void println(char* buf) {
	print(newln(buf, strlen(buf)));
}

void printnum(int num) {
	char buf[BUFFER_LENGTH];
	
	itoa(num, buf);

	println(buf);	
}

void pause() {
	char b[10];
	
	print("Press enter to continue...");
	
	read(STD_IN, b, 10);
}

int get_argc(char* args) {
	int r = 0, i = 0;

	while(strncmp(args + i, "PATH=", 5)) {
		while(args[i++]);
		r++;
	}
	
	return r;
}

char* get_argv(char* args, int argnum) {
	static int __offset[] = {5, 4, 6, 5, 2};
	int i = 0, __tmp;
	
	if(argnum >= 0) while(argnum--) {
		while(args[i++]);
	}else {
		__tmp = argnum = -argnum - 1;
		
		while(strncmp(args + i, "PATH=", 5)) while(args[i++]);
		
		while(argnum--) while(args[i++]);
		
		return args + i + __offset[__tmp];
	}

	return args + i;
}

void checkarg(char* args, int n) {
	if(get_argc(args) <= n) {
		printerr(EARGC);
		_exit(EARGC);
	}
}

void printerr(int code) {
	println(errtxt[code]);
}

void checkerr() {
	if(errno) {
		printerr(errno);
		_exit(errno);
	}
}

#endif
#endif
