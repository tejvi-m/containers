#include <stdio.h>
#define _GNU_SOURCE
#include <sched.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/mount.h>
#include <signal.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include <getopt.h>


struct child{
  char *hostname;
  char **args;        // command to be executed + other args
  int pipe_fd[2];    // for communication between parent and child
}

/* to clone stack for the clone call */
struct stackClone{
  char stack[4096];
  char ptr[0];
}

static int runContainer(void arg*){
  
}

int main(int argc, char* argv[]){
    int flags = SIGCHILD | CLONE_NEWUTS;
    // sigchild - termination signal
    // clone newuts - namespace for unix time sharing. set a new host name.
    // TODO add more flags for more namespaces.

    pid_t container;

    struct child childProc;
    struct clone_stack cloneStack;

    childProc.hostname = "container";
    childProc.args = &argv[2];
    // optind - from getOpt.h
    // index of the next element thats yet to be processed in argv.

    container = clone(runContainer, stack.ptr, flags, &childProc);
    // clone - sched.h
    // TODO read more of the manpage

    if(container = -1) die("clone");

    printf("child created with PID: %ld\n", (long) container);

    close(childProc.pipe_fd[1]);

    if(waitpid(container, NULL, 0) == -1) die("waitpid");

    printf("terminating\n");

    exit(EXIT_SUCCESS);

}
