#include <string.h>
#include "utils.h"

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
	buf[read(0, buf, BUFFER_LENGTH) - 1] = STRING_END;
}

int scannum() {
	char buf[BUFFER_LENGTH];

	scan(buf);

	return atoi(buf);
}

void print(char* buf) {
	int len = strlen(buf);

	buf[len] = STRING_END;
	
	write(1, buf, len + 1);
}

void println(char* buf) {
	print(newln(buf, strlen(buf)));
}

void printnum(int num) {
	char buf[BUFFER_LENGTH];
	
	itoa(num, buf);

	println(buf);	
}
