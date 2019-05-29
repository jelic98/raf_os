#define __LIBRARY__
#include <unistd.h>
#include <crypt.h>

_syscall3(int,keyset,const char*,key,int,length,int,local)
_syscall1(int,keyclear,int,local)
_syscall1(int,keygen,int,level)
_syscall1(int,catchkey,int,catch)
_syscall1(int,copykey,char*,key)
_syscall3(int,encr,char*,file,int,length,int,scall)
_syscall3(int,decr,char*,file,int,length,int,scall)
_syscall1(int,ignorecrypt,int,ignore)
_syscall1(int,uisencr,int,fd)
_syscall3(int,getkey,char*,key,int,local,int,scall)
_syscall1(int,encrlst,int,fd)
_syscall1(int,decrlst,int,fd)

int keymatch(int inum, char* key) {
	int i;

	for(i = 0; i < LST_MAXLEN; i++) {
		if(*enc_list[i].inum == inum) {
			char tmp[KEY_MAXLEN];
			strcpy(tmp, key);
			hash(tmp);

			return !strcmp(enc_list[i].key, tmp);
		}
	}

	return 0;
}
