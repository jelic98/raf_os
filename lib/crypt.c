#include <crypt.h>

void ienclst() {
	int fd = open(ENC_LST, O_RDWR | O_CREAT | O_EXCL, S_IRWXU);
	
	if(fd > 0) {
		close(fd);
	}

	errno = 0;
}

void renclst() {
	ienclst();
	
	static int readcnt = 0;

	if(readcnt++) {
		return;
	}

	int fd = open(ENC_LST, O_RDONLY);
	
	if(fd > 0) {	
		int len;

		do {
			char* path;

			len = read(fd, path, FLNM_MAXLEN);

			int enc = open(path, O_RDONLY);

			if(enc > 0) {
				encrlst(enc, path, strlen(path));
				close(enc);
			}
		}while(len);

		close(fd);
	}
	
	errno = 0;
}

void wenclst() {
	ienclst();
	
	int fd = open(ENC_LST, O_WRONLY);
	
	if(fd > 0) {
		int i = 0;
		
		while(i++ < FDSC_MAXLEN) {
			char* ent;

			lstent(i, ent);

			write(fd, ent, FLNM_MAXLEN);
		}
		
		close(fd);
	}

	errno = 0;
}

int isencr(int fd) {
	return enclst[fd][0];
}
