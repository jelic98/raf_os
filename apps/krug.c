static int r;

void unos_krug() {
	r = scannum();
}

int obim_krug() {
	int obim;

	__asm__ __volatile__ (
		"imull $2, %1;"
		"imull $3, %1;"
		"movl %1, %0;"
		: "=r" (obim)
		: "r" (r)
	);

	return obim;
}

int povrsina_krug() {
	int povrsina;

	__asm__ __volatile__ (
		"imull %1, %1;"
		"imull $3, %1;"
		"movl %1, %0;"
		: "=r" (povrsina)
		: "r" (r)
	);

	return povrsina;
}
