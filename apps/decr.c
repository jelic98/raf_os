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
		//if(!uisencr(fd)) {
		//	printerr(EALDECR);
		//	close(fd);
		//	_exit(EALDECR);
		//}

		char buf[BLOCK_LENGTH];
		int blen;

		while((blen = read(fd, buf, BLOCK_LENGTH)) > 0) {
			decr(buf, blen);
			checkerr();

			strncat(file, buf, blen);

			memset(buf, 0, sizeof(buf));
		};

		close(fd);
	}else {
		println(MSG_FILE_ERROR);
	}

	fd = open(path, O_WRONLY);
	
	if(fd > 0) {
		write(fd, file, strlen(file));
		decrlst(fd, path, strlen(path));
		close(fd);
	}else {
		println(MSG_FILE_ERROR);
	}

	wenclst();

	_exit(0);
}
