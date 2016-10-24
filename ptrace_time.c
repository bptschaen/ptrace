/*
 * Interecepts calls to get time of day.
 *
 * There seem to be multiple ways to get the time including:
 * (i) gettimeofday
 * (ii) opening /etc/localtime
 *
 *
 */


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
#include <string.h>
#include <errno.h>

char *get_string(pid_t child, unsigned long addr){
    /* from nelhage/ministrace */
    char *val = malloc(4096);
    int allocated = 4096;
    int read = 0;
    unsigned long tmp;
    while (1) {
        if (read + sizeof tmp > allocated) {
            allocated *= 2;
            val = realloc(val, allocated);
        }
        tmp = ptrace(PTRACE_PEEKDATA, child, addr + read);
        if(errno != 0) {
            val[read] = 0;
            break;
        }
        memcpy(val + read, &tmp, sizeof tmp);
        if (memchr(&tmp, 0, sizeof tmp) != NULL)
            break;
            read += sizeof tmp;
        }
        return val;
}


void getdata(pid_t child, long addr,
                     char *str, int len)
{
    int long_size = sizeof(long);
    char *laddr;
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
        child, addr + i * 4, NULL);
        memcpy(laddr, data.chars, long_size);
        ++i;
        laddr += long_size;
    }
    j = len % long_size;
    if(j != 0) {
        data.val = ptrace(PTRACE_PEEKDATA, child, addr + i * 4, NULL);
        memcpy(laddr, data.chars, j);
    }
    str[len] = '\0';
}


void watch_child(pid_t child){
    struct user_regs_struct regs;
    int status;
    while(1) {
        wait(&status);
        if(WIFEXITED(status)){
            break;
        }
        ptrace(PTRACE_GETREGS, child, NULL, &regs);
        //printf( "sys_call: %llu \n", regs.orig_rax );
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
        } else if(regs.orig_rax == SYS_open) {
            char *filename1 = get_string(child, regs.rdi);
            if( strcmp(filename1, "/etc/localtime")==0 ){
                printf( "open(\"%s\")\n", filename1 );
            }
            free(filename1);
        } else {
            //Do nothing
        }
 
        //actuallly run system call
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
