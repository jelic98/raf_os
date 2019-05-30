#ifndef _CRYPT_H
#define _CRYPT_H

#define KEY_MAXLEN 16
#define ENT_MAXLEN 19
#define LST_MAXLEN 5
#define PAD_CHR '~'
#define KEY_LVLS {1, 2, 3}
#define FLNM_MAXLEN 32
#define FDSC_MAXLEN 32
#define LOCAL_TIMEOUT 45*HZ
#define GLOBAL_TIMEOUT 120*HZ

#include <errno.h>
#include <fcntl.h>
#include <linux/sched.h>
#include <linux/fs.h>
#include <utils.h>

#define keyok(x) (x[0] != 0)
#define keylen(x) (1 << (x + 1))
#define keylenok(x) ((x > 0) && (x < KEY_MAXLEN) && ((x & (x - 1)) == 0))
#define keytype current->local_key[0]

typedef struct enc_entry {
	unsigned char* inum;
	char* encrypted;
	char* key;
} enc_entry;

char gkey[KEY_MAXLEN];
enc_entry enc_list[LST_MAXLEN];
long global_timeout;
char tmpkey[KEY_MAXLEN];
int tmpindex;
int catchkey;

void encrypt(char* args);
void encrstr(char* s);
void decrstr(char* s);
void crypt(char* args, int encrypt);
int keymatch(int inum, char* key);

#endif
