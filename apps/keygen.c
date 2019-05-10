#include <unistd.h>
#include <string.h>
#include <crypt.h>

#define H_UTILS_IMPLEMENT
#include "utils.h"

int main(char* args) {
	checkarg(args, 1);
	
	renclst();
	
	char* key = get_argv(args, 1);

	keygen(atoi(key));

	checkerr();

	_exit(0);
}
