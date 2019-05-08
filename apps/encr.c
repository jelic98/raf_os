#include <unistd.h>
#include <string.h>

#define H_UTILS_IMPLEMENT
#include "utils.h"

int main(char* args) {
	checkarg(args, 1);

	char* file = get_argv(args, 1);

	encr(file, strlen(file));

	checkerr();
}
