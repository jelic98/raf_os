#include <hash.h>

void hash(char* s) {
	compute(s, HASH_OFFSET);
}

void dehash(char* s) {
	compute(s, -HASH_OFFSET);
}

static void compute(char* s, int offset) {
	while(*s) {
		*s += offset;

		if(*s > 126) {
			*s = 32 + *s - 126;
		}else if(*s < 32) {
			*s = 126 - 32 + *s;
		}

		s++;
	}
}
