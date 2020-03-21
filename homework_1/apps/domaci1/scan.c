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

	// AX = auxiliary (esi --> ax --> edi)
	// BX = output counter
	// CX = scancode
	// DL = flags
	// ESI = input scancodes/mnemonics
	// EDI = output buffer
	
	// flags = 00000SCA
	// S = shift (1 down, 0 up)
	// C = ctrl (1 down, 0 up)
	// A = alt (1 down, 0 up)

	// scancodes = aAbBcC1!2@3#...
	// even positions = small caps
	// odd positions = big caps
	// easier for getting big caps from small caps

	// mnemonics = a #00 b #01 c #02 ...
	// index 	 = 0  1  2  3  4  5  ...
	// where #xx is pointer to string
	// chars are places inside 4 bytes because pointers are int

	// pipeline
	// get small caps --> check shift --> check ctrl --> check alt --> exit

	__asm__ __volatile__ (	
		"jmp START;"

		// PRINT BEGIN
		
		"PRINT:" // get char from scancode and add it to output buffer
		"pushl %%ecx;" // save scancode for later
		"incl %%ecx;" // count from 1 not from 0
		"cld;"
		"rep;" // ecx holds scancode
		"lodsw;" // load small caps and big caps into ax
		"stosb;" // copy small caps from al to edi
		"popl %%ecx;" // retrieve saved scancode
		"ret;"
		
		"SHIFT_PRINT:" // replace char in output buffer with big caps
		"movb -1(%%esi), %%al;" // esi points to next small caps, copy previous big caps to al
		"movb %%al, -1(%%edi);" // edi points to address after last char in output
		"jmp CTRL_CHECK;"

		"CTRL_PRINT:" // replace char in output buffer with mapped string
		"movl %4, %%esi;" // esi points to mnemonics instead of scancodes (possible problem?)

		"CTRL_LOOP:" // find mnemonic
		"cmpb (%%esi), %%al;" // al holds char from input
		"je CTRL_LOOP_AFTER;"
		"addl $0x8, %%esi;" // increment esi by 2 int slots in mnemonics array
		"jmp CTRL_LOOP;"

		"CTRL_LOOP_AFTER:" // prepare for string manipulation
		"addl $0x4, %%esi;" // increment esi by 1 int slot to make it point to pointer to string
		"movl (%%esi), %%esi;" // double pointer, load string address in esi
		"xorl %%ecx, %%ecx;" // reset ecx which will hold string length

		"CTRL_LENGTH:" // calculate string length
		"lodsb;" // copy char from string to al
		"cmpb $0x0, %%al;" // check for end in zero terminated string
		"je CTRL_LENGTH_AFTER;"
		"incl %%ecx;" // increment ecx which will hold string length
		"jmp CTRL_LENGTH;"

		"CTRL_LENGTH_AFTER:" // print mapped string
		"decl %%edi;" // replace last printed char which is mapped to string
		"subl %%ecx, %%esi;" // esi points to starting address of string 
		"decl %%esi;" // count from 0 not from 1
		"cld;"
		"rep;" // ecx holds string length
		"movsb;" // move ecx chars from string to output buffer
		"jmp ALT_CHECK;"
		
		"ALT_PRINT:" // replace output buffer with alt char
		"movb $0xA, %%al;" // al = 10
		"imulb %8;" // al = 10 * alt char
		"movb %%al, %8;" // alt char = 10 * alt char
		"decl %%edi;" // make edi point to last printed char
		"movb (%%edi), %%al;" // copy last printed char to al
		"subb $0x30, %%al;" // assembly version of atoi :)
		"addb %%al, %8;" // append last printed char as digit to alt char
		"movb %%al, (%%edi);" // remove last printed char
		"jmp EXIT;"
		
		// PRINT END

		// KEY DOWN BEGIN
		
		"SHIFT_DOWN:"
		"orb $0x4, %%dl;" // set shift bit to 1
		"jmp EXIT;"
		
		"CTRL_DOWN:"
		"orb $0x2, %%dl;" // set ctrl bit to 1
		"jmp EXIT;"
		
		"ALT_DOWN:"
		"orb $0x1, %%dl;" // set alt bit to 1
		"jmp EXIT;"	

		// KEY DOWN END
		
		// KEY UP BEGIN

		"SHIFT_UP:"
		"andb $0x3, %%dl;" // set shift bit to 0
		"jmp EXIT;"
		
		"CTRL_UP:"
		"andb $0x5, %%dl;" // set ctrl bit to 0
		"jmp EXIT;"
	
		"ALT_UP:"
		"movb %8, %%al;" // move alt code from input to al
		"cld;"
		"stosb;" // move alt code from al to output buffer
		"movb $0x0, %8;" // reset alt code, must be 0 for next time
		"andb $0x6, %%dl;" // set alt bit to 0
		"jmp EXIT;"
		
		// KEY UP END
		
		// START BEGIN

		"START:"
		"movl %%edi, %%ebx;" // save output buffer address to calculate number of prited chars
		
		"cmpl $"CODE_END", %%ecx;" // check terminating condition
		"je EXIT;"

		"cmpl $"SHIFT_DOWN", %%ecx;" // check if shift is pressed
		"je SHIFT_DOWN;"

		"cmpl $"CTRL_DOWN", %%ecx;" // check if ctrl is pressed
		"je CTRL_DOWN;"

		"cmpl $"ALT_DOWN", %%ecx;" // check if alt is pressed
		"je ALT_DOWN;"
		
		"cmpl $"SHIFT_UP", %%ecx;" // check if shift is released
		"je SHIFT_UP;"
		
		"cmpl $"CTRL_UP", %%ecx;" // check if ctrl is released
		"je CTRL_UP;"
		
		"cmpl $"ALT_UP", %%ecx;" // check if alt is released
		"je ALT_UP;"	

		"call PRINT;" // start pipeline

		// START END

		// CHECK BEGIN
		
		"SHIFT_CHECK:" // check if shift is pressed while reding scancode
		"pushw %%dx;" // save flags
		"andb $0x4, %%dl;" // shift bit is at index 2 (2^2 = 4)
		"cmpb $0x0, %%dl;" // check if bit is on
		"popw %%dx;" // retrieve flags
		"jne SHIFT_PRINT;"
		
		"CTRL_CHECK:" // check if ctrl is pressed while reding scancode
		"pushw %%dx;" // save flags
		"andb $0x2, %%dl;" // ctrl bit is at index 1  (2^1 = 2)
		"cmpb $0x0, %%dl;" // check if bit is on
		"popw %%dx;" // retrieve flags
		"jne CTRL_PRINT;"

		"ALT_CHECK:" // check if alt is pressed while reding scancode
		"pushw %%dx;" // save flag
		"andb $0x1, %%dl;" // alt bit is at index 0 (2^0 = 1)
		"cmpb $0x0, %%dl;" // check if bit is on
		"popw %%dx;" // retrieve flags
		"jne ALT_PRINT;"
		
		// CHECK END
	
		"EXIT:" // output buffer is ready, must save alt code and calculate printed chars
		"movb %8, %%al;" // copy alt code from input to al
		"movb %%al, %2;" // copy alt code from al to output
		"subl %%edi, %%ebx;" // ebx = address of output buffer, edi = address of last char in buffer
		"negl %%ebx;" // number of printed chars must be positive (edi > ebx)
		
		: "=b" (result), "=d" (flags), "=g" (alt)
		: "S" (scancodes), "g" (mnemonics), "D" (buffer), "c" (scancode), "d" (flags), "g" (alt)
		: "eax"
	);

	return result;
}
