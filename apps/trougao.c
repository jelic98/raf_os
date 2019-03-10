static int a;
static int b;
static int c;

void unos_trougao() {
	a = scannum();
	b = scannum();
	c = scannum();
}

int obim_trougao() {
	int obim;

	__asm__ __volatile__ (
		"addl %1, %2;"
		"addl %2, %3;"
		"movl %3, %0;"
		: "=r" (obim)
		: "r" (a), "r" (b), "r" (c)
	);

	return obim;
}

int povrsina_trougao() {
	int povrsina;

	__asm__ __volatile__ (
		"imull %2, %1;"
		"movl $0x0, %%edx;"
		"movl %1, %%eax;"
		"movl $2, %%ebx;"
		"idivl %%ebx;"
		"movl %%eax, %0;"
		: "=r" (povrsina)
		: "r" (a), "r" (b)
		: "%ebx"
	);

	return povrsina;
}
