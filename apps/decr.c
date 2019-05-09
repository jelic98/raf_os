#include <unistd.h>
#include <string.h>
#include <crypt.h>

#define H_UTILS_IMPLEMENT
#include "utils.h"

int main(char* args) {
	checkarg(args, 1);
	
	char file[BLOCK_LENGTH];
	
	char* path = get_argv(args, 1);

	if(path[0] != '/') {
		char* pwd = get_argv(args, ARG_PWD);
		
		if(strlen(pwd) > 1) {
			strcat(pwd, "/");
		}

		strcat(pwd, path);
		strcpy(path, pwd);
	}

	int fd = open(path, O_RDONLY);

	if(fd > 0) {
		fgets(file, BLOCK_LENGTH, fd);

		close(fd);
	}else {
		println(MSG_FILE_ERROR);
		resterr();
	}
	
	decr(file, strlen(file));

	fd = open(path, O_WRONLY);
	
	if(fd > 0) {
		*enclst[fd] = 0;

		fputs(file, BLOCK_LENGTH, fd);

		close(fd);
	}else {
		println(MSG_FILE_ERROR);
		resterr();
	}

	wenclst();
	resterr();

	checkerr();
}
