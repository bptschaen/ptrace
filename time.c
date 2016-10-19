
#include <stdio.h>
#include <time.h>
#include <sys/time.h>


void print_time(){
    time_t t = time(NULL);
    t = time(NULL);
    t = time(NULL);
    t = time(NULL);
    struct tm tm = *localtime(&t);
    printf("%d-%d-%d\n", tm.tm_year+1900, tm.tm_mon+1, tm.tm_mday);
}

int main(){
    pid_t child;
    child = fork();
    if(child == 0) {
        ptrace(PTRACE_TRACEME, 0, NULL, NULL); 
        print_time();
    }
    while(1) {
        wait(&status);
        if(WIFEXITED(status)){ break; }
        ptrace(PTRACE_GETREGS, child, NULL, &regs);
        if(regs.orig_rax == SYS_write) {
            printf("system write");
        }
 
}

