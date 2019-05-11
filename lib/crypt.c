#include <crypt.h>

// Check if file is encrypted - kernel space
int isencr(int inum) {	
	return *(enclst + inum);
}
