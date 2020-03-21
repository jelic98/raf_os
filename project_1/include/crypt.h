#ifndef _CRYPT_H
#define _CRYPT_H

#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#define KEY_MAXLEN 128
#define ASCII_FIRST 32
#define ASCII_LAST 126
#define PAD_CHR '~'
#define LST_START '!'
#define KEY_LVLS {1, 2, 3}
#define FLNM_MAXLEN 32
#define FDSC_MAXLEN 32

#define keyok(x) (x[0] != 0)
#define keylen(x) (1 << (x + 1))
#define keylenok(x) ((x > 0) && (x < KEY_MAXLEN) && ((x & (x - 1)) == 0))
#define isascii(x) ((x >= ASCII_FIRST) && (x <= ASCII_LAST))

char gkey[KEY_MAXLEN];
char* enclst;
int igncry;

int isencr(int inum);

#endif
