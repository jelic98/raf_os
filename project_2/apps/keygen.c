#include <crypt.h>

int main(char* args) {
	checkarg(args, 1);
	
	char* level = get_argv(args, 1);
	
	keygen(atoi(level));
	
	checkerr();
	
	_exit(0);
}
