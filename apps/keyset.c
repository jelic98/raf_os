#include <unistd.h>
#include <string.h>
#include <crypt.h>

#define H_UTILS_IMPLEMENT
#include "utils.h"

int main(char* args) {
	checkarg(args, 1);

	int fd = open(ENC_LST, O_RDWR | O_CREAT | O_EXCL, S_IRWXU);
	
	if(fd > 0) {
		close(fd);
	}else {
		// TODO Load file in memory
	}

	resterr();

	char* key = get_argv(args, 1);

	keyset(key, strlen(key));
	
	checkerr();
}
