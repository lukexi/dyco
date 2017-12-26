#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
int main(void) {
    // Pipe[0] is Out, Pipe[1] is In (0ut & 1n)
    int StdInPipe[2];  pipe(StdInPipe);
    int StdOutPipe[2]; pipe(StdOutPipe);
    int StdErrPipe[2]; pipe(StdErrPipe);

    pid_t ChildPID = fork();
    if(ChildPID == -1) {
        perror("fork");
        exit(1);
    }

    if(ChildPID == 0) {
        close(StdInPipe[1]);
        close(StdOutPipe[0]);
        close(StdErrPipe[0]);
        int Err = 0;
        // Err = dup2(fileno(stdin),  fd[0]); if (Err == -1) printf("dup2 err: %i\n", Err);
        Err = dup2(StdInPipe[0],  fileno(stdin));  if (Err == -1) printf("dup2 err: %i\n", Err);
        Err = dup2(StdOutPipe[1], fileno(stdout)); if (Err == -1) printf("dup2 err: %i\n", Err);
        Err = dup2(StdErrPipe[1], fileno(stderr)); if (Err == -1) printf("dup2 err: %i\n", Err);

        const char* Cmd = "/bin/cat";
        Err = execl(Cmd, Cmd, "-", NULL);
        if (Err) printf("Err %i\n", Err);
        exit(0);
    }
    else {
        close(StdInPipe[0]);
        close(StdOutPipe[1]);
        close(StdErrPipe[1]);

        char InputString[] = "What's up";
        write(StdInPipe[1], InputString, strlen(InputString)+1);

        /* Read in a string from the pipe */
        char ReadBuffer[80];
        int NumBytes = read(StdOutPipe[0], ReadBuffer, 80);
        if (NumBytes) {
            printf("Received string: %s", ReadBuffer);
        }
        else {
            printf("No\n");
        }
    }

    return(0);
}
