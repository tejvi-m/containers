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


#define die(msg)                                      \
do {                                                  \
    perror(msg);                                      \
    exit(EXIT_FAILURE);                               \
} while (0)

struct child{
  char *hostname;
  char **args;        // command to be executed + other args
  int pipe_fd[2];    // for communication between parent and child
};

/* to clone stack for the clone call */
struct stackClone{
  char stack[4096];
  char ptr[0];
};

static int runContainer(void *arg){
  struct child *args = (struct child *) arg;
  char ch;
  int err = 0;

  close(args->pipe_fd[1]);

  if (read(args->pipe_fd[0], &ch, 1) != 0) {
      die("Failure in child: read from pipe returned != 0\n");
  }


  sethostname(args -> hostname, strlen(args -> hostname));

  if (umount2("/proc", MNT_DETACH) < 0) die("unmount proc");
  // unmount proc, mnt_detach - lazy unmount. -makes mount point unavailable for new accesses, and actually umount when its no longer busy.
  // umount2 has more flags than umount
  // https://linux.die.net/man/2/umount2

  if (mount("proc", "/proc", "proc", 0, NULL) < 0) die("mount proc");

  printf("Executing %s\n", args->args[0]);

  err = execvp(args->args[0], args->args);

  if (0 != err) {
      printf("execvp error: %s\n", strerror(errno));
  }
}

int main(int argc, char* argv[]){
    int flags = SIGCHLD | CLONE_NEWUTS | CLONE_NEWPID;
    // sigchild - termination signal
    // clone newuts - namespace for unix time sharing. set a new host name.
    // TODO add more flags for more namespaces.

    pid_t container;

    struct child childProc;
    struct stackClone cloneStack;

    childProc.hostname = "container";
    childProc.args = &argv[1];

    if (pipe(childProc.pipe_fd) == -1) die("pipe");

    container = clone(runContainer, cloneStack.ptr, flags, &childProc);
    // clone - sched.h
    // TODO read more of the manpage

    if(container == -1) die("clone");

    printf("child created with PID: %ld\n", (long) container);

    close(childProc.pipe_fd[1]);

    if(waitpid(container, NULL, 0) == -1) die("waitpid");

    printf("terminating\n");

    exit(EXIT_SUCCESS);

}
