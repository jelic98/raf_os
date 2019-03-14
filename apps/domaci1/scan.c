#include "scan.h"

static char scancodes[SCANCODES_LENGTH * 2 + 1] = {0};
static int mnemonics[MNEMONICS_LENGTH * 2] = {0};
static char alt = 0;
static char flags = 0;

void load_config(const char* scancodes_filename, const char* mnemonics_filename) {
	int scancodes_file = fopen(scancodes_filename);

	char line1[SCANCODES_LENGTH + 1];
	char line2[SCANCODES_LENGTH];

	fgets(line1, SCANCODES_LENGTH, scancodes_file);
	fgets(line2, SCANCODES_LENGTH, scancodes_file);
	
	int i = -1;	
	char c;

	while(i++, line1[i] != '\n') {
		if(line1[i] == '"') {
			line1[i] == '\"';
		}
		
		if(line2[i] == '"') {
			line2[i] == '\"';
		}

		scancodes[2 * i] = line1[i];
		scancodes[2 * i + 1] = line2[i];
	}
	
    scancodes[2 * i] = '\0';

	fclose(scancodes_file);

	int mnemonics_file = fopen(mnemonics_filename);
	int maxlen = MNEMONIC_TEXT_LENGTH + 2;

	char tmp[3];
	fgets(tmp, 3, mnemonics_file);
	int n = atoi(tmp);

	for(i = 0; i < n; i++) {
		char buf[maxlen];
		int len = fgets(buf, maxlen, mnemonics_file);
		buf[len - 1] = 0;

		mnemonics[2 * i] = buf[0];
		mnemonics[2 * i + 1] = (int) buf + 2;
	
		println(buf + 2);
	}

	fclose(mnemonics_file);
}

int process_scancode(int scancode, char* buffer) {
	int result;

	__asm__ __volatile__ (	
		"cmpl $"SHIFT_DOWN", %%ecx;"
		"je KEY_DW;"
		
		"cmpl $"SHIFT_UP", %%ecx;"
		"je KEY_UP;"
		
		"cmpl $"CTRL_DOWN", %%ecx;"
		"je KEY_DW;"
		
		"cmpl $"CTRL_UP", %%ecx;"
		"je KEY_UP;"
		
		"cmpl $"ALT_DOWN", %%ecx;"
		"xorb %bl, %bl"
		"je KEY_DW;"
	
		"cmpl $"ALT_UP", %%ecx;"
		"je KEY_UP;"	

		"SETUP:"
		"incl %%ecx;"
		"xorl %%ebx, %%ebx;"

		"RESET_OUT:"
		"movl %1, %%edi;"

		"GET_CHAR:" // u ecx je sc
		"cld;"
		"rep;"
		"lodsb;"
		"stosb;"
		"ret;" // u edi je pocetak chara

		"PRINT_CHAR:" // u esi je pocetak inputa
		"cmpb $0x0, (%%esi);"
		"je EXIT;"
		"movsb;"
		"incl %%ebx;"
		"jmp PRINT_CHAR;" // u ebx je result
		
		"CTRL_PRINT:" // u edi je pocetak chara, u esi je pocetak mnemonika
		"cmpb (%%edi), (%%esi);"
		"je MNEM_FOUND;"
		"addl $8, %%esi;"
		"jmp CTRL_PRINT;"
		"incl %%esi;"
		"jmp PRINT_CHAR;"

		"SHIFT_PRINT:" // u edi je pocetak small caps
		"incb %%edi;"
		"movl %%edi, %%esi;"
		"movb $0x0, 1(%%esi);"
		"jmp PRINT_CHAR;"

		"ALT_PRINT:" // u bl je alt char, u al je ucitana cifra
		"shlb %%bl;"
		"orb %%al, %%bl;"
		"movb %%bl, (alt);"
		"jmp EXIT;"

		"EXIT:"
		: "=b" (result)
		: "S" (scancodes), "D" (buffer), "c" (scancode)
		: "eax", "edx"
	);

	return result;
}
