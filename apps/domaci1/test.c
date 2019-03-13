#include <stdio.h>
#include "scan.h"

int main(int argc, char** argv) {
	char src[5] = "LAZR";
	char dst[5];
	int c = 2;
	int result = 0;
	
	__asm__ __volatile__ (
		"cmpl "SHIFT_UP", %%ecx;"
		"je SHI_UP;"
		
		"cmpl "SHIFT_DOWN", %%ecx;"
		"je SHI_DW;"

		"cmpl "CTRL_UP", %%ecx;"
		"je CTR_UP;"

		"cmpl "CTRL_DOWN", %%ecx;"
		"je CTR_DW;"

		"cmpl "ALT_UP", %%ecx;"
		"je ALT_UP;"

		"cmpl "ALT_DOWN", %%ecx;"
		"je ALT_DW;"

		"PRINT:"
		"incl %%ecx;"
		"cld;"
		"rep;"
		"lodsb;"
		"stosb;"
		"incl %%ebx;"
		"jmp EXIT;"

		"SHI_UP:"
		"SHI_DW:"
		"CTR_UP:"
		"CTR_DW:"
		"ALT_UP:"
		"ALT_DW:"

		"EXIT:"

		: "=b" (result)
		: "S" (src), "D" (dst), "c" (c)
	);

	printf("RESULT: %d OUTPUT: %s\n", result, dst);

	return 0;
}
