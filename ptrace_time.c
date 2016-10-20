#include <time.h>
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

void watch_child(pid_t child){
    struct user_regs_struct regs;
    int status;
    while(1) {
        wait(&status);
        if(WIFEXITED(status)){
            break;
        }
        ptrace(PTRACE_GETREGS, child, NULL, &regs);
        //printf( "regs.orig_rax: %llu \n", regs.orig_rax);
        //printf( "regs.rax: %llu \n", regs.rax);
        if(regs.orig_rax == SYS_write) {
            printf("Sys_writing\n");
        } else if(regs.orig_rax == SYS_gettimeofday) {
            printf("gettimeoftime\n");
        } else if(regs.orig_rax == SYS_time) {
            printf("time\n");
        } else if(regs.orig_rax == SYS_times) {
            printf("times\n");
        } else if(regs.orig_rax == SYS_utime) {
            printf("utime\n");
        } else if(regs.orig_rax == SYS_utimes) {
            printf("utimes\n");
        } else if(regs.orig_rax == SYS_clock_gettime) {
            printf("clock_gettime\n");
        } else {
            //Do nothing
        }
 
        ptrace(PTRACE_SYSCALL, child, NULL, NULL);
      }

}

int main()
{
   pid_t child;
   child = fork();
   if(child == 0) {
      ptrace(PTRACE_TRACEME, 0, NULL, NULL);
      if( execl("/home/liedetector/ptrace/time", "time", (char *)NULL)==-1){
        printf("process failed\n");
      }
   }
   else {
      watch_child(child);
   }
   return 0;
}
