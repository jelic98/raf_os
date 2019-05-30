#include <string.h>
#include <crypt.h>

int main(char* args) {
	char key[KEY_MAXLEN];
	
	keycatch(1), pause_input(), keycopy(key), keycatch(0);

	keyset(key, strlen(key), 0);
		
	checkerr();

	_exit(0);
}
