#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
int main(int argc, char* argv[]) {
    int fds[2];
    /* Spawn a process from pfunc, returning it's pid. The fds array passed will
     * be filled with two descriptors: fds[0] will read from the child process,
     * and fds[1] will write to it.
     * Similarly, the child process will receive a reading/writing fd set (in
     * that same order) as arguments.
    */
    printf("stdin: %i\n", fileno(stdin));
    printf("stdout: %i\n", fileno(stdout));
    printf("stderr: %i\n", fileno(stderr));

    int pipes[4];

    /* Warning: I'm not handling possible errors in pipe/fork */

    pipe(&pipes[0]); /* Parent read/child write pipe */
    pipe(&pipes[2]); /* Child read/parent write pipe */

    pid_t pid = fork();
    if (pid > 0) {
        /* Parent process */
        fds[0] = pipes[0];
        fds[1] = pipes[3];

        char ProcessOut[1024];
        printf("Reading...\n");
        int Err = read(pipes[0], ProcessOut, 1024);
        printf("%i Buffer: %s\n", Err, ProcessOut);

        close(pipes[1]);
        close(pipes[2]);

        return pid;

    } else {
        close(pipes[0]);
        close(pipes[3]);

        dup2(fileno(stdin), pipes[2]);
        dup2(fileno(stdout), pipes[1]);
        dup2(fileno(stderr), pipes[1]);
        int Result = execl("/bin/echo", "foo");
        printf("Hello %i\n", Result);

        // pfunc(pipes[2], pipes[1]);

        exit(0);
    }

    return 0;
}
