#define __LIBRARY__
#include <unistd.h>
#include <crypt.h>

_syscall3(int,keyset,const char*,key,int,length,int,local)
_syscall1(int,keyclear,int,local)
_syscall1(int,keygen,int,level)
_syscall1(int,keycatch,int,catch)
_syscall1(int,keycopy,char*,key)
_syscall3(int,keyget,char*,key,int,local,int,scall)
_syscall3(int,encr,int,fd,char*,path,int,len)
_syscall3(int,decr,int,fd,char*,path,int,len)

void encrstr(char* s, char* key) {
	int length = strlen(s);
	length -= length < BLOCK_SIZE;		

	int i, j, k;

	int m = strlen(key);
    int n = length / m + (length % m != 0);

	char hed[m];
	strcpy(hed, key);

	char shed[m];
	strcpy(shed, key);
	
    char mat[n][m];
    char copmat[n][m];
	
	char tmp;
	
	for(k = 0; k < m - 1; k++) {
		for(j = 0; j < m - k - 1; j++) {
			if(shed[j] > shed[j + 1]) {
				tmp = shed[j];
				shed[j] = shed[j + 1];
				shed[j + 1] = tmp;
			}
		}	
	}
	
	for(i = 0; i < n; i++) {
        for(j = 0; j < m; j++) {
            k = i * m + j;

			if(k < length && isascii(s[k])) {
                mat[i][j] = s[k];
            }else {
                mat[i][j] = PAD_CHR;
			}

			copmat[i][j] = mat[i][j];
        }
	}

	for(k = 0; k < m; k++) {
		for(j = 0; j < m; j++) {
			if(j != k && hed[k] == shed[j]) {
				for(i = 0; i < n; i++) {
					mat[i][j] = copmat[i][k];
				}
			}
		}
	}

	memset(copmat, 0, sizeof(copmat));

	for(k = 0, j = 0; j < m; j++) {
		for(i = 0; i < n; i++) {
			s[k++] = mat[i][j];
		}
	}
}

void decrstr(char* s, char* key) {
	int length = strlen(s);
	length -= length < BLOCK_SIZE;		
	
	int i, j, k;
	
    int m = strlen(key);
    int n = length / m + (length % m != 0);

	char hed[m];
	strcpy(hed, key);

	char shed[m];
	strcpy(shed, key);
	
    char mat[n][m];
    char copmat[n][m];
	
	char tmp;

	for(k = 0; k < m - 1; k++) {
		for(j = 0; j < m - k - 1; j++) {
			if(shed[j] > shed[j + 1]) {
				tmp = shed[j];
				shed[j] = shed[j + 1];
				shed[j + 1] = tmp;
			}
		}	
	}
	
	for(i = 0; i < n; i++) {
        for(j = 0; j < m; j++) {
            k = j * n + i;

			copmat[i][j] = mat[i][j] = s[k];
        }
	}

	for(k = 0; k < m; k++) {
		for(j = 0; j < m; j++) {
			if(j != k && hed[k] == shed[j]) {
				for(i = 0; i < n; i++) {
					mat[i][k] = copmat[i][j];
				}
			}
		}
	}

	memset(copmat, 0, sizeof(copmat));

	for(k = 0, i = 0; i < n; i++) {
        for(j = 0; j < m; j++) {
			if(mat[i][j] != PAD_CHR) {
                s[k++] = mat[i][j];
			}
        }
	}
}

void crypt(char* args, int encrypt) {
	checkarg(args, 1);

	char path[FLNM_MAXLEN];
	get_path(args, path);
	
	int fd = open(path, O_RDONLY);

	if(fd > 0) {
		if(encrypt) {
			encr(fd, path, strlen(path));
		}else {
			decr(fd, path, strlen(path));
		}

		checkerr();
		close(fd);
	}else {
		println(MSG_FILE_ERROR);
	}
}

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
