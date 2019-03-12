#define H_UTILS_IMPLEMENT
#include "../utils.h"
#include "scan.h"

void main(int argc, char** argv) {
	load_config(FILE_SCANCODES, FILE_MNEMONICS);

	while(1) {
		char filename[FILENAME_LENGTH];

		print(MSG_FILE_NAME);
		scan(filename);
		
		if(!strcmp(filename, CMD_EXIT)) {
			break;	
		}

		int file = fopen(filename);

		int code;
		char buf[CODE_LENGTH + 1];
	
		do {
			fgets(buf, CODE_LENGTH + 1, file), 
			
			code = atoi(buf);
	
			char out[OUTPUT_LENGTH] = {0};
			
			process_scancode(code, out);

			print(out);
		}while(code != FILE_END);

		print(NEW_LINE_STRING);

		fclose(file);
	}
	
	_exit(0);
}
