#include <unistd.h>
#include <string.h>

#define H_UTILS_IMPLEMENT
#include "utils.h"

int main(char* args) {
	checkarg(args, 1);

	char file[BLOCK_LENGTH];
	
	char* path = get_argv(args, 1);

	int fd = open(path, O_RDONLY, 0);
	
	if(fd > 0) {
		fgets(file, BLOCK_LENGTH, fd);

		close(fd);
	}else {
		println(MSG_FILE_ERROR);
		resterr();
	}

	encr(file, strlen(file));

	fd = open(path, O_WRONLY, 0);
	
	if(fd > 0) {
		fputs(file, BLOCK_LENGTH, fd);

		close(fd);
	}else {
		println(MSG_FILE_ERROR);
		resterr();
	}	

	checkerr();
}
