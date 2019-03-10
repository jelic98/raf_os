static int a;

void unos_kvadrat() {
	a = scannum();
}

int obim_kvadrat() {
	int obim;

	__asm__ __volatile__ (
		"imull $4, %1;"
		"movl %1, %0;"
		: "=r" (obim)
		: "r" (a)
	);

	return obim;
}

int povrsina_kvadrat() {
	int povrsina;

	__asm__ __volatile__ (
		"imull %1, %1;"
		"movl %1, %0;"
		: "=r" (povrsina)
		: "r" (a)
	);

	return povrsina;
}
