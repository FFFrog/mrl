//**********************************************************************************************
//
//    Filename:         fork.c
//    Author:           lijiawei
//    Create at:        2021/06/07 15:15:07
//    Description:      fork
//
//**********************************************************************************************

#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>

int main()
{
    pid_t pid = fork();
    if (pid == 0) {
        printf("child\n");
        execl("/bin/ls", "ls", NULL);
    } else {
        wait(NULL);
        printf("father\n");
    }

    return 0;
}
