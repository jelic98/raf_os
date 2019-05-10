#define __LIBRARY__
#include <unistd.h>

_syscall3(int,decrlst,int,fd,char*,path,int,length)
