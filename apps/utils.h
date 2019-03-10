#ifndef H_UTILS
#define H_UTILS

#define STD_IN 0
#define STD_OUT 1
#define STD_ERROR 2

#define BUFFER_LENGTH 128
#define ASCII_OFFSET 48
#define STRING_END '\0'
#define NEW_LINE '\n'
#define PI 3.14

#define __isdigit(x) ((x >= '0') && (x <= '9'))

int digitcount(int n);

int itoa(int n, char* buf);
int atoi(const char* buf);

void reverse(char* buf, int len);
char* newln(char* buf, int len);

void scan(char* buf);
int scannum();

void print(char* buf);
void println(char* buf);
void printnum(int n);

#endif
