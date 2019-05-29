#ifndef _UTILS_H
#define _UTILS_H

#define ASCII_OFFSET 48
#define __isdigit(x) ((x >= '0') && (x <= '9'))

int digitcount(int n);
void reverse(char* buf, int len);
int itoa(int n, char* buf);
int atoi(const char* buf);

#endif
