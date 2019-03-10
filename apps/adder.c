#include <string.h>
#include <unistd.h>

#define STD_IN 0
#define STD_OUT 1
#define STD_ERR 2

#define BUF_LEN 128
#define ASC_OFF 48
#define STR_END '\0'
#define NEW_LN '\n'

int strtonum(char* str) {
	int num = 0;
	char chr;

	while(chr = *(str++), chr >= '0' && chr <= '9') {
		num *= 10;
		num += chr - ASC_OFF;
	}

	return num;
}

void numtostr(int num, char* str, int len) {
	while(num > 0) {
		str[--len] = num % 10 + ASC_OFF;		
		num /= 10;
	}
}

int digcnt(int num) {
	int cnt = -1;

	while(cnt++, num > 0) {
		num /= 10;
	}

	return cnt;
}

char* newln(char* str, int len) {
	str[len] = NEW_LN;

	return str;
}

int scan(char* buf) {
	int len = read(0, buf, BUF_LEN);

	buf[len - 1] = STR_END;

	return len;
}

void print(char* buf, int len) {
	buf[len] = STR_END;
	write(1, buf, len + 1);
}

void println(char* buf, int len) {
	print(newln(buf, len), len + 1);
}

void main(int argc, char** argv) {
	char buf[BUF_LEN];

	scan(buf);

	int one = strtonum(buf);
	int two = strtonum(buf + digcnt(one) + 1);

	int sum = one + two;
	int len = digcnt(sum);

	numtostr(sum, buf, len);
	
	println(buf, len);

	_exit(0);
}
