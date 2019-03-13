#include "scan.h"

static char scancodes[2][SCANCODES_LENGTH] = {0};
static int mnemonics[MNEMONICS_LENGTH][2] = {0};

void load_config(const char* scancodes_filename, const char* mnemonics_filename) {
	int scancodes_file = fopen(scancodes_filename);

	fgets(scancodes[0], SCANCODES_LENGTH, scancodes_file);
	fgets(scancodes[1], SCANCODES_LENGTH, scancodes_file);

	fclose(scancodes_file);

	int mnemonics_file = fopen(mnemonics_filename);
	int maxlen = MNEMONIC_TEXT_LENGTH + 2;
	
	char buf[maxlen];
	fgets(buf, maxlen, mnemonics_file);
	int n = atoi(buf);
	int i;

	for(i = 0; i < n; i++) {
		int len = fgets(buf, maxlen, mnemonics_file);
		buf[len - 1] = 0;

		mnemonics[i][0] = buf[0];
		mnemonics[i][1] = (int) buf + 2;
	}

	fclose(mnemonics_file);
}

int process_scancode(int scancode, char* buffer) {
	int result;

	__asm__ __volatile__ (	
		"SPEC:"
		"cmpl $"SHIFT_DOWN", %%ecx;"
		"je KEY_DW;"
		"cmpl $"SHIFT_UP", %%ecx;"
		"je KEY_UP;"
		"cmpl $"CTRL_DOWN", %%ecx;"
		"je KEY_DW;"
		"cmpl $"CTRL_UP", %%ecx;"
		"je KEY_UP;"
		"cmpl $"ALT_DOWN", %%ecx;"
		"je KEY_DW;"
		"cmpl $"ALT_UP", %%ecx;"
		"je KEY_UP;"	

		"KEY_DW:"
		"jmp EXIT;"

		"KEY_UP:"
		"jmp EXIT;"

		"SETUP:"
		"incl %%ecx;"
		"xorl %%ebx, %%ebx;"

		"PRINT:"
		"call MODE;"
		"cld;"
		"rep;"
		"lodsb;"
		"stosb;"
		"incl %%ebx;"
		"jmp EXIT;"

		"MODE:"
		"popl %%edx;"
		"incl %%ebx;"
		"cmpl $"SHIFT_DOWN", %%edx;"
		"je PRINT;"
		"cmpl $"CTRL_DOWN", %%edx;"
		"incl %%ebx;"
		"je PRINT;"
		"cmpl $"ALT_DOWN", %%edx;"
		"incl %%ebx;"
		"je PRINT;"
		"pushl %%edx;"
		"ret;"
		
		"EXIT:"
		: "=b" (result)
		: "S" (scancodes), "D" (buffer), "c" (scancode)
		: "eax", "edx"
	);

	return result;
}
