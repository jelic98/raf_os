#include <errno.h>
#include <string.h>
#include <unistd.h>

#define UTIL_IMPLEMENTATION
#include "utils.h"

int main(int argc, char* argv[]) {
	char* file = "test.txt";
	char* key = "abcd";
	int level = 2;

	int ret = keyset(key);
	
	vardump(ret);
	vardump(errno);

	_exit(0);
}
