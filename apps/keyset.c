#include <unistd.h>
#include <string.h>
#include <crypt.h>

#define H_UTILS_IMPLEMENT
#include "utils.h"

int main(char* args) {
	char key[KEY_MAXLEN];
	
	catchkey(1), pause(), copykey(key), catchkey(0);

	keyset(key, strlen(key), 0);
		
	checkerr();

	_exit(0);
}
