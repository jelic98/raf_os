#ifndef SCAN_H_INCLUDED
#define SCAN_H_INCLUDED

#define SHIFT_DOWN "200"
#define SHIFT_UP "300"
#define CTRL_DOWN "201"
#define CTRL_UP "301"
#define ALT_DOWN "202"
#define ALT_UP "302"
#define CODE_END "400"

#define CMD_EXIT "exit"

#define MSG_CHAR_COUNT "Character count: "

#define SCANCODES_LENGTH 128
#define MNEMONICS_LENGTH 16
#define MNEMONIC_TEXT_LENGTH 64
#define CODE_LENGTH 3
#define OUTPUT_LENGTH 128
#define FILENAME_LENGTH 32

#define FILE_SCANCODES "scancodes.tbl"
#define FILE_MNEMONICS "ctrl.map"
#define FILE_TEST "test1.tst"

void load_config(const char* scancodes_filename, const char* mnemonics_filename);
int process_scancode(int scancode, char* buffer);

#endif
