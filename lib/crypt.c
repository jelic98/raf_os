#include <crypt.h>

void ienclst() {
	int fd = open(ENC_LST, O_RDWR | O_CREAT | O_EXCL, S_IRWXU);
	
	if(fd > 0) {
		close(fd);
	}
}

void renclst() {
	int fd = open(ENC_LST, O_RDONLY);
	
	if(fd > 0) {	
		int len;

		do {
			char* path;

			len = read(fd, path, FLNM_MAXLEN);

			int enc = open(path, O_RDONLY);

			if(enc > 0) {
				strcpy(enclst[enc], path);
				close(enc);
			}
		}while(len);

		close(fd);
	}else {
		ienclst();
	}
}

void wenclst() {
	int fd = open(ENC_LST, O_WRONLY);
	
	if(fd > 0) {
		int i = 0;
		
		while(i++ < FDSC_MAXLEN) {
			write(fd, enclst[i], FLNM_MAXLEN);
		}
		
		close(fd);
	}else {
		ienclst();
	}
}

int isencr(int fd) {
	return enclst[fd][0];
}
