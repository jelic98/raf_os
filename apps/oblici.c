#include <unistd.h>
#include <string.h>
#include "utils.h"
#include "oblici.h"

static void (*f_unos[])() = {unos_trougao, unos_kvadrat, unos_krug};

static int (*f_racunanje[][2])() = {
	{obim_trougao, povrsina_trougao},
	{obim_kvadrat, povrsina_kvadrat},
	{obim_krug, povrsina_krug}
};

void main(int argc, char** argv) {
	int opt;

	while(1) {
		print(MSG_MENU);
		opt = scannum();

		if(opt-- == 0) {
			break;
		}

		if(opt < 0 || opt > 2) {
			println(MSG_ERROR);
			continue;
		}

		f_unos[opt]();

		printnum(f_racunanje[opt][0]());
		printnum(f_racunanje[opt][1]());
	}
	
	_exit(0);
}
