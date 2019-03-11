#define H_UTILS_IMPLEMENT
#include "../utils.h"

#define NAME_LENGTH 32

#define FILE_SCANCODES "scancodes.txt"
#define FILE_MNEMONICS "mnemonics.txt"

void main(int argc, char** argv) {
	char filename[NAME_LENGTH];

	print(MSG_FILE_NAME);
	scan(filename);
	int file = fopen(filename);

	char buf[BUFFER_LENGTH];	
	fgets(buf, file);
	
	int n = atoi(buf);
	int array[50];
	int i;

	for(i = 0; i < n; i++) {
		fgets(buf, file);
		array[i] = atoi(buf);
	}

	fclose(file);
	_exit(0);
}
