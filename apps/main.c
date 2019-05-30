#include <string.h>
#include <crypt.h>

int main(char* args) {
	char gkey[] = "1234";
	keyset(gkey, strlen(gkey), 0);
	
	char lkey[] = "5678";
	keyset(lkey, strlen(lkey), 1);
	
	while(1) {
		char global[KEY_MAXLEN];
		keyget(global, 0, 1);
		print(global);

		char local[KEY_MAXLEN];
		keyget(local, 1, 1);
		print(local);
	
		pause_input();
	}
		
	_exit(0);
}
