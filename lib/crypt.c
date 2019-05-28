#define __LIBRARY__
#include <unistd.h>

_syscall3(int,keyset,const char*,key,int,length,int,local)
_syscall1(int,keyclear,int,local)
_syscall1(int,keygen,int,level)
_syscall1(int,catchkey,int,catch)
_syscall1(int,copykey,char*,key)
_syscall3(int,encr,char*,file,int,length,int,scall)
_syscall3(int,decr,char*,file,int,length,int,scall)
_syscall1(int,ignorecrypt,int,ignore)
_syscall1(int,uisencr,int,fd)
_syscall2(int,getkey,char*,key,int,local)
_syscall3(int,encrlst,int,fd,char*,path,int,length)
_syscall3(int,decrlst,int,fd,char*,path,int,length)
