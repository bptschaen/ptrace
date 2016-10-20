
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <stdio.h>

int main(){
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    printf("%d-%d-%d\n", tm.tm_year+1900, tm.tm_mon+1, tm.tm_mday);

}

