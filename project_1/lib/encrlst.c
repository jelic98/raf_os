#define __LIBRARY__
#include <unistd.h>

_syscall3(int,encrlst,int,fd,char*,path,int,length)
