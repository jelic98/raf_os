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
	}

	fclose(mnemonics_file);
}

int process_scancode(int scancode, char* buffer) {
	int result;
	
	// flags = 00000SCA
	// S = shift (1 down, 0 up)
	// C = ctrl (1 down, 0 up)
	// A = alt (1 down, 0 up)

	__asm__ __volatile__ (	
		"jmp START;"

		"SHIFT_DOWN:"
		"orb $0x4, %%dl;"
		"jmp EXIT;"
		
		"CTRL_DOWN:"
		"orb $0x2, %%dl;"
		"jmp EXIT;"
		
		"ALT_DOWN:"
		"orb $0x1, %%dl;"
		"jmp EXIT;"	

		"SHIFT_UP:"
		"andb $0x3, %%dl;"
		"jmp EXIT;"
		
		"CTRL_UP:"
		"andb $0x5, %%dl;"
		"jmp EXIT;"
	
		"ALT_UP:"
		"andb $0x6, %%dl;"
		"jmp EXIT;"

		"START:"
		"xorl %%ebx, %%ebx;"

		"cmpl $"SHIFT_DOWN", %%ecx;"
		"je SHIFT_DOWN;"

		"cmpl $"CTRL_DOWN", %%ecx;"
		"je CTRL_DOWN;"

		"cmpl $"ALT_DOWN", %%ecx;"
		"je ALT_DOWN;"
		
		"cmpl $"SHIFT_UP", %%ecx;"
		"je SHIFT_UP;"
		
		"cmpl $"CTRL_UP", %%ecx;"
		"je CTRL_UP;"
		
		"cmpl $"ALT_UP", %%ecx;"
		"je ALT_UP;"	
/*
		"RESET_OUT:"
		"movl %1, %%edi;"

		"GET_CHAR:" // u ecx je sc
		"cld;"
		"rep;"
		"lodsb;"
		"stosb;"
		"ret;" // u edi je pocetak chara

		"incl %%ecx;"
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

		"ALT_PRINT:" // u bh je alt char, u al je ucitana cifra
		"shlb %%bh;"
		"orb %%al, %%bh;"
		"movb %%bh, (alt);"
		"jmp EXIT;"
*/
		"EXIT:"
		
		: "=b" (result), "=d" (flags)
		: "S" (scancodes), "D" (buffer), "c" (scancode), "d" (flags)
		: "eax"
	);

	printnum(flags);

	return result;
}
