#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>

int main(void)
{    
    // create child
    pid_t child = fork();

    switch (child)
    {
        // error
        case -1:
            printf("fork failed\n");
            break;

        // child
        case 0:
            pid_t id = getpid();
            
            // command to execute
            char *argv_list[2];
            argv_list[0] = "./process";
            argv_list[1] = NULL;

            execvp(argv_list[0], argv_list);
            break;

        // parent
        default:
            sleep(5);

            // wait for child to receive signal
            kill(child, SIGINT);

            printf("program completed\n");
            break;
    }
    return 0;
}
