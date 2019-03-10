#ifndef H_OBLICI
#define H_OBLICI

#define MSG_ERROR "Opcija nije ispravna"
#define MSG_MENU "Izbor oblika\n\
1 - TROUGAO\n\
2 - KVADRAT\n\
3 - KRUG\n\
0 - IZLAZ\n\
Izbor: "

void unos_trougao();
int obim_trougao();
int povrsina_trougao();

void unos_kvadrat();
int obim_kvadrat();
int povrsina_kvadrat();

void unos_krug();
int obim_krug();
int povrsina_krug();

#endif
