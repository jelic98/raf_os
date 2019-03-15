#include "scan.h"

static char scancodes[SCANCODES_LENGTH * 2 + 1] = {0};
static int mnemonics[MNEMONICS_LENGTH * 2];
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
	char buf[3][maxlen];

	for(i = 0; i < n; i++) {
		int len = fgets(buf[i], maxlen, mnemonics_file);
		buf[i][len - 1] = '\0';

		mnemonics[2 * i] = buf[i][0];
		mnemonics[2 * i + 1] = (int) buf[i] + 2;
	}

	fclose(mnemonics_file);
}

int process_scancode(int scancode, char* buffer) {
	int result;

	// AH = auxiliary
	// BX = output counter
	// CX = scancode
	// DL = flags
	// ESI = input scancodes/mnemonics
	// EDI = output buffer
	
	// flags = 00000SCA
	// S = shift (1 down, 0 up)
	// C = ctrl (1 down, 0 up)
	// A = alt (1 down, 0 up)

	__asm__ __volatile__ (	
		"jmp START;"

		// PRINT BEGIN
		
		"PRINT:"
		"pushl %%ecx;"
		"incl %%ecx;"
		"cld;"
		"rep;"
		"lodsw;"
		"stosb;"
		"popl %%ecx;"
		"ret;"
		
		"SHIFT_PRINT:"
		"movb -1(%%esi), %%al;"
		"movb %%al, -1(%%edi);"
		"jmp CTRL_CHECK;"

		"CTRL_PRINT:"
		"movl %4, %%esi;"

		"CTRL_LOOP:"
		"cmpb (%%esi), %%al;"
		"je CTRL_LOOP_AFTER;"
		"addl $0x8, %%esi;"
		"jmp CTRL_LOOP;"

		"CTRL_LOOP_AFTER:"
		"addl $0x4, %%esi;"
		"movl (%%esi), %%esi;"
		"xorl %%ecx, %%ecx;"

		"CTRL_LENGTH:"
		"lodsb;"
		"cmpb $0x0, %%al;"
		"je CTRL_LENGTH_AFTER;"
		"incl %%ecx;"
		"jmp CTRL_LENGTH;"

		"CTRL_LENGTH_AFTER:"
		"decl %%edi;"
		"subl %%ecx, %%esi;"
		"decl %%esi;"
		"cld;"
		"rep;"
		"movsb;"
		"jmp ALT_CHECK;"
		
		"ALT_PRINT:"
		"movb $0xA, %%al;"
		"imulb %8;"
		"movb %%al, %8;"
		"decl %%edi;"
		"movb (%%edi), %%al;"
		"subb $0x30, %%al;"
		"addb %%al, %8;"
		"xorb %%al, %%al;"
		"movb %%al, (%%edi);"
		"jmp EXIT;"
		
		// PRINT END

		// KEY DOWN BEGIN
		
		"SHIFT_DOWN:"
		"orb $0x4, %%dl;"
		"jmp EXIT;"
		
		"CTRL_DOWN:"
		"orb $0x2, %%dl;"
		"jmp EXIT;"
		
		"ALT_DOWN:"
		"orb $0x1, %%dl;"
		"jmp EXIT;"	

		// KEY DOWN END
		
		// KEY UP BEGIN

		"SHIFT_UP:"
		"andb $0x3, %%dl;"
		"jmp EXIT;"
		
		"CTRL_UP:"
		"andb $0x5, %%dl;"
		"jmp EXIT;"
	
		"ALT_UP:"
		"movb %8, %%al;"
		"cld;"
		"stosb;"
		"xorb %%al, %%al;"
		"andb $0x6, %%dl;"
		"jmp EXIT;"
		
		// KEY UP END
		
		// START BEGIN

		"START:"
		"movl %%edi, %%ebx;"
		
		"cmpl $"CODE_END", %%ecx;"
		"je EXIT;"

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

		"call PRINT;"
		
		// START END

		// CHECK BEGIN
		
		"SHIFT_CHECK:"
		"pushw %%dx;"
		"andb $0x4, %%dl;"
		"cmpb $0x0, %%dl;"
		"popw %%dx;"
		"jne SHIFT_PRINT;"
		
		"CTRL_CHECK:"
		"pushw %%dx;"
		"andb $0x2, %%dl;"
		"cmpb $0x0, %%dl;"
		"popw %%dx;"
		"jne CTRL_PRINT;"

		"ALT_CHECK:"
		"pushw %%dx;"
		"andb $0x1, %%dl;"
		"cmpb $0x0, %%dl;"
		"popw %%dx;"
		"jne ALT_PRINT;"
		
		// CHECK END
	
		"EXIT:"
		"movb %8, %%al;"
		"movb %%al, %2;"
		"subl %%edi, %%ebx;"
		"negl %%ebx;"
		
		: "=b" (result), "=d" (flags), "=g" (alt)
		: "S" (scancodes), "g" (mnemonics), "D" (buffer), "c" (scancode), "d" (flags), "g" (alt)
		: "eax"
	);

	return result;
}
