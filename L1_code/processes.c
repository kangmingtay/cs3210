/*******************************************************************
* processes.c
* Demonstrates process creation in linux.
* Compile: gcc -o processes processes.c
* Run: ./processes
******************************************************************/

#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int value = 10;

int main(int argc, char* argv[])
{
    pid_t fork_ret;
    
    // creating the child process
    fork_ret = fork();
 	   
    if (fork_ret < 0) {
        // fork_ret would be -1 if unsuccessful
        fprintf(stderr, "Fork Failed\n");

        return 1;
    } else if (fork_ret == 0) {
        // fork_ret would return 0 in child
        printf("We just cloned a process..!\n");
        value += 10;
        printf("Child process: value = %d\n", value);
    } else {
        // parent waits until child is completed
        wait(NULL);
        printf("Child Completed ....\n");
        printf("Parent process: value = %d\n", value);
    }

    return 0;
}
