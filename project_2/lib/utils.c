#include <errno.h>
#include <fcntl.h>
#include <utils.h>

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

	for(i = 0; isdigit(buf[i]); ++i) {
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

void pause_input() {
	char b[1];
	
	read(STD_IN, b, 1);
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

	println(errtxt[code]);
}

void checkerr() {
	if(errno) {
		printerr(errno);
		_exit(errno);
	}
}

void get_path(char* args, char* res) {
	char* path = get_argv(args, 1);

	if(path[0] != '/') {
		char* pwd = get_argv(args, ARG_PWD);
		
		if(strlen(pwd) > 1) {
			strcat(pwd, "/");
		}

		strcat(pwd, path);
		strcpy(res, pwd);
	}else {
		strcpy(res, path);
	}
}
