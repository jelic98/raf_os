#include <string.h>
#include "utils.h"

int digcnt(int num) {
	int cnt = -1;

	while(cnt++, num > 0) {
		num /= 10;
	}

	return cnt;
}

int strtonum(char* str) {
	int num = 0;
	char chr;

	while(chr = *(str++), chr >= '0' && chr <= '9') {
		num *= 10;
		num += chr - ASC_OFF;
	}

	return num;
}

void numtostr(int num, char* str) {
	int len = digcnt(num);

	while(num > 0) {
		str[--len] = num % 10 + ASC_OFF;		
		num /= 10;
	}
}

char* newln(char* str, int len) {
	str[len] = NEW_LN;

	return str;
}

void scan(char* buf) {
	int len = read(0, buf, BUF_LEN);

	buf[len - 1] = STR_END;
}

int scannum() {
	char buf[BUF_LEN];

	scan(buf);

	return strtonum(buf);
}

void print(char* buf) {
	int len = strlen(buf);

	buf[len] = STR_END;
	
	write(1, buf, len + 1);
}

void println(char* buf) {
	int len = strlen(buf);
	
	print(newln(buf, len));
}

void printnum(int num) {
	char buf[BUF_LEN];
	
	numtostr(num, buf);

	println(buf);	
}
