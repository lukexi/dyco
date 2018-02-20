#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <spawn.h>

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

    posix_spawn_file_actions_t SpawnActions;
    posix_spawn_file_actions_init(&SpawnActions);
    posix_spawn_file_actions_adddup2(&SpawnActions, StdInPipe[0], fileno(stdin));
    posix_spawn_file_actions_addclose(&SpawnActions, StdInPipe[1]);

    posix_spawn_file_actions_adddup2(&SpawnActions, StdOutPipe[1], fileno(stdout));
    posix_spawn_file_actions_adddup2(&SpawnActions, StdErrPipe[1], fileno(stderr));
    posix_spawn_file_actions_addclose(&SpawnActions, StdOutPipe[0]);
    posix_spawn_file_actions_addclose(&SpawnActions, StdErrPipe[0]);

    int PID;
    posix_spawnp(&PID, Args[0], &SpawnActions, NULL, Args, NULL);
    posix_spawn_file_actions_destroy(&SpawnActions);

    close(StdInPipe[0]);
    close(StdOutPipe[1]);
    close(StdErrPipe[1]);

    if (StdIn != NULL) {
        write(StdInPipe[1], StdIn, strlen(StdIn));
    }
    close(StdInPipe[1]);

    // Wait for the child process to complete
    int ExitStatus;
    while (waitpid(PID, &ExitStatus, WNOHANG) == 0) {
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
