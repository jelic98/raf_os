#ifndef H_UTILS_INCLUDE
#define H_UTILS_INCLUDE

#include <unistd.h>
#include <string.h>
#include <fcntl.h>

#define STD_IN 0
#define STD_OUT 1
#define STD_ERROR 2

#define BUFFER_LENGTH 128
#define ASCII_OFFSET 48
#define STRING_END '\0'
#define NEW_LINE '\n'
#define NEW_LINE_STRING "\n"

#define MSG_FILE_NAME "Unesite naziv fajla: "
#define MSG_FILE_ERROR "Greska pri otvaranju fajla\n"

#define __isdigit(x) ((x >= '0') && (x <= '9'))

#define vardump(x) {\
	char buff[128];\
	int len;\
	len = itoa(x, buff);\
	write(STD_OUT, #x ": ", strlen(#x ": "));\
	write(STD_OUT, buff, len);\
	write(STD_OUT, "\n", 1);\
}

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

void print(char* buf);
void println(char* buf);
void printnum(int n);

void pause(void);

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

#endif
#endif
