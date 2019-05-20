#include <unistd.h>
#include <string.h>
#include <crypt.h>

#define H_UTILS_IMPLEMENT
#include "utils.h"

void get_path(char* args, char* res) {
	char* path = get_argv(args, 1);

	if(path[0] != '/') {
		char* pwd = get_argv(args, ARG_PWD);
		
		if(strlen(pwd) > 1) {
			strcat(pwd, "/");
		}

		strcat(pwd, path);
		strcpy(res, pwd);
	}else {
		strcpy(res, path);
	}
}

int main(char* args) {
	checkarg(args, 1);
	
	char file[FILE_LENGTH];

	char path[FLNM_MAXLEN];
	get_path(args, path);
	
	int fd = open(path, O_RDONLY);

	if(fd > 0) {
		if(uisencr(fd)) {
			printerr(EALENCR);
			close(fd);
			_exit(EALENCR);
		}
		
		char buf[BLOCK_LENGTH];
		int blen;

		while((blen = read(fd, buf, BLOCK_LENGTH)) > 0) {
			encr(buf, blen, 1);
			checkerr();

			strncat(file, buf, blen);

			memset(buf, 0, sizeof(buf));
		};

		close(fd);
	}else {
		println(MSG_FILE_ERROR);
	}
	
	file[strlen(file) - 1] = 0;

	fd = open(path, O_WRONLY);
	
	if(fd > 0) {
		write(fd, file, strlen(file));
		encrlst(fd, path, strlen(path));
		close(fd);
	}else {
		println(MSG_FILE_ERROR);
	}

	_exit(0);
}
