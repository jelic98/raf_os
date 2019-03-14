#define H_UTILS_IMPLEMENT
#include "../utils.h"
#include "scan.h"

#define DEBUG 1

static const char scancodes_filename[FILENAME_LENGTH] = FILE_SCANCODES;
static const char mnemonics_filename[FILENAME_LENGTH] = FILE_MNEMONICS;

void main(int argc, char** argv) {
	load_config(scancodes_filename, mnemonics_filename);

	while(1) {
		char filename[FILENAME_LENGTH] = FILE_TEST;

		if(!DEBUG) {
			print(MSG_FILE_NAME);
			scan(filename);
		
			if(!strcmp(filename, CMD_EXIT)) {
				break;	
			}
		}

		int file = fopen(filename);

		int code;
		char buf[CODE_LENGTH + 1];
	
		do {
			fgets(buf, CODE_LENGTH + 1, file), 
			
			code = atoi(buf);

			char out[OUTPUT_LENGTH] = {0};
			
			int result = process_scancode(code, out);

			print(out);
		}while(code != FILE_END);

		print(NEW_LINE_STRING);

		fclose(file);

		if(DEBUG) break;
	}
	
	_exit(0);
}
