#ifndef H_UTILS
#define H_UTILS

#define STD_IN 0
#define STD_OUT 1
#define STD_ERR 2

#define BUF_LEN 128
#define ASC_OFF 48
#define STR_END '\0'
#define NEW_LN '\n'
#define PI 3.14

int digcnt(int num);
int strtonum(char* str);
void numtostr(int num, char* str);
char* newln(char* str, int len);
void scan(char* buf);
int scannum();
void print(char* buf);
void println(char* buf);
void printnum(int num);

#endif
