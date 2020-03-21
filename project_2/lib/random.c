#include <random.h>

static unsigned long int next;

int rand() {
	if(!next) {
		next = CURRENT_TIME;
	}

	next = next * 1103515245 + 12345;
	
	return next % 65535;
}
