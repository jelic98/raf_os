#include <string.h>
#include <unistd.h>
#include "utils.c"

void main(int argc, char** argv) {
	char buf[BUF_LEN];

	scan(buf);

	int one = atoi(buf);
	int two = atoi(buf + digcnt(one) + 1);

	printnum(one + two);

	_exit(0);
}
