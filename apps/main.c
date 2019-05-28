#include <unistd.h>
#include <string.h>
#include <crypt.h>

#define H_UTILS_IMPLEMENT
#include "utils.h"

int main(char* args) {
	char gkey[] = "1234";
	keyset(gkey, strlen(gkey), 0);
	
	char lkey[] = "5678";
	keyset(lkey, strlen(lkey), 1);
	
	while(1) {
		char global[KEY_MAXLEN];
		getkey(global, 0);
		print(global);

		char local[KEY_MAXLEN];
		getkey(local, 1);
		print(local);
	
		pause();
	}
		
	_exit(0);
}
