#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <spawn.h>
#include <sys/wait.h>

extern char **environ;
void run_cmd(char *cmd)
{
    pid_t pid;
    char *argv[] = {(char*)"SimpleGtkProgressBar",NULL};
    int status;
    printf("Run command: %s\n", cmd);
    status = posix_spawn(&pid, "SimpleGtkProgressBar", NULL, NULL, argv, environ);
    if (status == 0) {
        printf("Child pid: %i\n", pid);
    } else {
        printf("posix_spawn: %s\n", strerror(status));
    }
    printf("\nEXITING");
    //sleep(100);
}

int main(int argc, char* argv[])
{
    run_cmd(argv[1]);
    return 0;
}