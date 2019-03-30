#include <unistd.h>
#define H_UTILS_IMPLEMENT
#include "utils.h"
void main() {
	char filename[128];
	print(MSG_FILE_NAME);
	scan(filename);
	println(filename);
	_exit(0);
}
