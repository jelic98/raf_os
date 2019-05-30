#ifndef _UTILS_H
#define _UTILS_H

#define STD_IN 0
#define STD_OUT 1
#define STD_ERROR 2

#define ARG_PAT -1
#define ARG_PWD -2
#define ARG_SHE -3
#define ARG_HOM -4
#define ARG_EXE -5

#define BLOCK_LENGTH 1024
#define FILE_LENGTH 2048
#define BUFFER_LENGTH 128
#define ASCII_OFFSET 48
#define ASCII_FIRST 32
#define ASCII_LAST 126
#define STRING_END '\0'
#define NEW_LINE '\n'
#define NEW_LINE_STRING "\n"
#define LINE_FEED 10

#define MSG_FILE_NAME "File name: \0"
#define MSG_FILE_ERROR "Cannot open selected file\0"

#define isascii(x) ((x >= ASCII_FIRST) && (x <= ASCII_LAST))
#define isdigit(x) ((x >= '0') && (x <= '9'))

#define vardump(x) {\
	char buf[BUFFER_LENGTH];\
	int len;\
	len = itoa(x, buf);\
	write(STD_OUT, #x ": ", strlen(#x ": "));\
	write(STD_OUT, buf, len);\
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
int fputs(char* buf, int len, int fd);

void print(char* buf);
void println(char* buf);
void printnum(int n);

void pause_input();

int get_argc(char *args);
char *get_argv(char *args, int argnum);

void checkarg(char* args, int n);
void printerr(int code);
void checkerr();

void get_path(char* args, char* res);

#endif
