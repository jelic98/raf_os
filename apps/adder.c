#define H_UTILS_IMPLEMENT
#include "utils.h"

void main(int argc, char** argv) {
	char buf[BUF_LEN];

	scan(buf);

	int one = atoi(buf);
	int two = atoi(buf + digitcount(one) + 1);

	printnum(one + two);

	_exit(0);
}
