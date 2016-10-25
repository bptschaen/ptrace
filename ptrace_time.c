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

void get_string(pid_t child, unsigned long addr, char *val){
    /* from nelhage/ministrace */
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
}

/**
 * rax is return call - 0 if success, -1 if failure
 * rdi is the timeval data struct
 */
void get_time(pid_t child, long long unsigned int rax, long long unsigned int rdi){
    if( rax!=0 ){
        return;
    }
    //TODO: get from central location
    ptrace(PTRACE_POKEDATA, child, rdi, 0);
}


void watch_child(pid_t child){
    struct user_regs_struct regs;
    int status;
    int entering=1;
    while(1) {
        wait(&status);
        if(WIFEXITED(status)){
            break;
        }
        ptrace(PTRACE_GETREGS, child, NULL, &regs);
        //printf( "sys_call: %llu \n", regs.orig_rax );
        switch( regs.orig_rax ){
            case SYS_write:
                break;
            case SYS_gettimeofday:{
                if( entering ){
                    printf("Entering ");
                    entering = 0;
                }
                else{
                    printf("Exiting ");
                    entering = 1;
                    get_time(child, regs.rax, regs.rdi );
                }
                break;
            }
            case SYS_open:{
                char *filename = malloc(4096);
                get_string(child, regs.rdi, filename);
                if( strcmp(filename, "/etc/localtime")==0 ){
                    //printf( "open(\"%s\")\n", filename );
                    //TODO: make sure this call is uniform
                }
                free(filename);
            }
            default:
                break;
        }
        //continue until next syscall entry/exit
        ptrace(PTRACE_SYSCALL, child, NULL, NULL);
      }

}

int main( int argc, char *argv[] )
{
    if( argc !=2 ){
        printf( "Usage %s <file_to_trace>\n", argv[0] );
        return 0;
    }
    char *program_to_run = argv[1];
    pid_t child;
    child = fork();
    if(child == 0) {
       ptrace(PTRACE_TRACEME, 0, NULL, NULL);
       printf( "Running: %s\n", program_to_run);
       if( execl( program_to_run, program_to_run, (char *)NULL)==-1){ //("/home/liedetector/ptrace/time"
         printf("process failed\n");
       }
    }
    else {
       watch_child(child);
    }
    return 0;
}
