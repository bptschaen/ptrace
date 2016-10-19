#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/user.h>
#include <sys/syscall.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
const int long_size = sizeof(long);
void reverse(char *str)
{   int i, j;
    char temp;
    for(i = 0, j = strlen(str) - 2;
        i <= j; ++i, --j) {
        temp = str[i];
        str[i] = str[j];
        str[j] = temp;
    }
}
void getdata(pid_t child, long addr,
             char *str, int len)
{   char *laddr;
    int i, j;
    union u {
            long val;
            char chars[long_size];
    }data;
    i = 0;
    j = len / long_size;
    laddr = str;
    while(i < j) {
        data.val = ptrace(PTRACE_PEEKDATA,
                          child, addr + i * 8,
                          NULL);
        memcpy(laddr, data.chars, long_size);
        ++i;
        laddr += long_size;
    }
    j = len % long_size;
    if(j != 0) {
        data.val = ptrace(PTRACE_PEEKDATA,
                          child, addr + i * 8,
                          NULL);
        memcpy(laddr, data.chars, j);
    }
    str[len] = '\0';
}
void putdata(pid_t child, long addr,
             char *str, int len)
{   char *laddr;
    int i, j;
    union u {
            long val;
            char chars[long_size];
    }data;
    i = 0;
    j = len / long_size;
    laddr = str;
    while(i < j) {
        memcpy(data.chars, laddr, long_size);
        ptrace(PTRACE_POKEDATA, child,
               addr + i * 8, data.val);
        ++i;
        laddr += long_size;
    }
    j = len % long_size;
    if(j != 0) {
        memcpy(data.chars, laddr, j);
        ptrace(PTRACE_POKEDATA, child,
               addr + i * 8, data.val);
    }
}

void dochild(){
  ptrace(PTRACE_TRACEME, 0, NULL, NULL);
  execl("/bin/ls", "ls", NULL);
}


void doparent(pid_t child){
  struct user_regs_struct regs;
  int status;
  char *str, *laddr;
  int toggle = 0;
  while(1) {
    wait(&status);
    if(WIFEXITED(status))
      break;
    ptrace(PTRACE_GETREGS, child, NULL, &regs);
    if(regs.orig_rax == SYS_write) {
       if(toggle == 0) {
          toggle = 1;
          str = (char *)calloc((regs.rdx+1),
   			  sizeof(char));
          getdata(child, regs.rsi, str,
   	       regs.rdx);
          reverse(str);
          putdata(child, regs.rsi, str,
   	       regs.rdx);
       }
       else {
          toggle = 0;
       }
    }
    ptrace(PTRACE_SYSCALL, child, NULL, NULL);
  }

}

int main()
{
  pid_t child;
  child = fork();
  if(child == 0) {
    dochild();
  }
  else {
    doparent(child);
   }
   return 0;
}
