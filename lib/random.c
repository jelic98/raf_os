extern long volatile jiffies;

static unsigned long int next;

int rand() {
	if(!next) {
		next = jiffies;
	}

	next = next * 1103515245 + 12345;
	
	return next % 65535;
}
