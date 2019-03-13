#define H_UTILS_IMPLEMENT
#include "utils.h"

int main(int argc, char *argv[]) {
	int array[5] = {1, 2, 3, 4, 5};
	int n = 5;
	int sum = 0;

	__asm__ __volatile__ (
		"xorl %%edx, %%edx;"
		"MORE: lodsl;"
		"addl %%eax, %%edx;"
		"loop MORE;"
		: "=d" (sum)
		: "S" (array), "c" (n)
		: "%eax"
	);

	printnum(sum);

	_exit(0);
}
