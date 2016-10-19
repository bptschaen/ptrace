#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/user.h>
//#include <linux/user.h>   /* For constants
//                                   ORIG_EAX etc */



//system calls can be found in /usr/include/x86_64-linux-gnu/asm/unistd_64.h


int main()
{   pid_t child;
    long orig_eax;
    child = fork();
    if(child == 0) {
        ptrace(PTRACE_TRACEME, 0, NULL, NULL);
        execl("/bin/ls", "ls", NULL);
    }
    else {
        wait(NULL);
        struct user_regs_struct regs;
        orig_eax = ptrace(PTRACE_GETREGS, child, NULL, &regs);
        printf("The child made a system call %ld\n", regs.orig_rax);
        ptrace(PTRACE_CONT, child, NULL, NULL);
    }
    return 0;
}
