#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>

int CallProcess(char* const* Args, const char* StdIn,
    char* OutBuffer, size_t OutBufferSize, size_t* OutLength,
    char* ErrBuffer, size_t ErrBufferSize, size_t* ErrLength)
{
    // Pipe[0] is Out, Pipe[1] is In (0ut & 1n)
    int StdInPipe[2];  pipe(StdInPipe);
    int StdOutPipe[2]; pipe(StdOutPipe);
    int StdErrPipe[2]; pipe(StdErrPipe);

    // Allow us to read from empty pipes
    // (We do a manual 'waitpid' to ensure they are ready to read from)
    fcntl(StdOutPipe[0], F_SETFL, O_NONBLOCK);
    fcntl(StdErrPipe[0], F_SETFL, O_NONBLOCK);

    pid_t ChildPID = fork();
    if(ChildPID == -1) {
        perror("fork");
        exit(1);
    }

    if (ChildPID == 0) {
        // Child process:
        // Close the sides of the pipes we won't be interacting with
        close(StdInPipe[1]);
        close(StdOutPipe[0]);
        close(StdErrPipe[0]);
        int Err = 0;
        Err = dup2(StdInPipe[0],  fileno(stdin));  if (Err == -1) printf("dup2 err: %i\n", Err);
        Err = dup2(StdOutPipe[1], fileno(stdout)); if (Err == -1) printf("dup2 err: %i\n", Err);
        Err = dup2(StdErrPipe[1], fileno(stderr)); if (Err == -1) printf("dup2 err: %i\n", Err);

        // v means pointer-to-args, p means use PATH environment variable
        Err = execvp(Args[0], Args);
        if (Err) printf("Error during execvp: %i\n", Err);
        exit(Err);
    }

    // Parent process:
    // Close the sides of the pipes we won't be interacting with
    close(StdInPipe[0]);
    close(StdOutPipe[1]);
    close(StdErrPipe[1]);

    // Write to StdIn
    if (StdIn != NULL) {
        write(StdInPipe[1], StdIn, strlen(StdIn));
    }
    close(StdInPipe[1]);

    // Wait for the child process to complete
    int ExitStatus;
    while (waitpid(ChildPID, &ExitStatus, WNOHANG) == 0) {
        // Wait for the process...
    }
    if (ExitStatus) {
        printf("Process exited with error status: %i\n", ExitStatus);
    }

    // Read StdOut/StdErr
    *OutLength = read(StdOutPipe[0], OutBuffer, OutBufferSize);
    *ErrLength = read(StdErrPipe[0], ErrBuffer, ErrBufferSize);
    close(StdOutPipe[0]);
    close(StdErrPipe[0]);

    // NULL-terminate
    const int LastOutChar = *OutLength < OutBufferSize ? *OutLength : (OutBufferSize-1);
    OutBuffer[LastOutChar] = '\0';
    const int LastErrChar = *ErrLength < ErrBufferSize ? *ErrLength : (ErrBufferSize-1);
    ErrBuffer[LastErrChar] = '\0';

    return ExitStatus;
}
