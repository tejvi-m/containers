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
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

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


void setupCgroups(pid_t containerPID, int memoryLimit, int cpuset, int procMax){

    // memory limit:
    // procMax: create new cgroup in /sys/fs/cgroup/pids/exp (exp is the cgroup name)
    // add current pid to the file cgroup.procs, and set procs.max to the maximum allowed number of processes.

    // for memory limit, /sys/fs/cgroup/memory/<pid>, cahnce memory.limit_in_bytes.



    //syntax for snprintf(char *, size, format, values...);

    // make a directory called exp in the groups pseudo fs

    char *memCgroupLimit = "/sys/fs/cgroup/memory/exp1/memory.limit_in_bytes";
    char *memCgroupPID = "/sys/fs/cgroup/memory/exp1/cgroup.procs";
    char *procCgroupMax = "/sys/fs/cgroup/pids/exp1/pids.max";
    char *procCgroupPID = "/sys/fs/cgroup/pids/exp1/cgroup.procs";

    // dont use getpid(), pids start at 1 now. use contianer pid

    mkdir("/sys/fs/cgroup/memory/exp1/", 0777);
    mkdir("/sys/fs/cgroup/pids/exp1/", 0777);

    FILE *fp1 = fopen(procCgroupPID, "a");
    fprintf(fp1 , "%d\n", (int) containerPID);
    fclose(fp1);

    FILE *fp2 = fopen(procCgroupMax, "w+");
    fprintf(fp2, "%d\n", procMax);
    fclose(fp2);

    int mem = 10;
    int pid = (int) containerPID;
    // memory limits are enforced but they are not shown by tools like top or free since they look at /proc/meminfo
    // we need to namespace proc/meminfo?
    FILE *fp3 = fopen(memCgroupPID, "a");
    fprintf(fp3, "%d\n", pid);
    fflush(fp3);
    fclose(fp3);

    FILE *fp4 = fopen(memCgroupLimit, "w+");
    fprintf(fp4, "%d\n", mem);
    fclose(fp4);

  
}

static int runContainer(void *arg){
  struct child *args = (struct child *) arg;
  char ch;
  int err = 0;

  close(args->pipe_fd[1]);

  if (read(args->pipe_fd[0], &ch, 1) != 0) {
      die("Failure in child: read from pipe returned != 0\n");
  }


  sethostname(args -> hostname, strlen(args -> hostname));
  if (mount("/", "/", "none", MS_PRIVATE | MS_REC, NULL) < 0 ) {
        die("mount private");
    }
  unshare(CLONE_NEWNS);

  // chaning root to a copy of the ubuntu file system.
  int env = chroot("~/rootfs/");
  if(env != 0) die("chroot");
  chdir("/");


  // in case of having a separate image (file system),
  // change root,
  // mount proc  and temporary file system
  // run command
  // unmount
  // changes do not persist, afaik


  // if (umount2("/proc", MNT_DETACH) < 0) die("unmount proc");
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
    int flags = SIGCHLD | CLONE_NEWUTS | CLONE_NEWPID | CLONE_NEWNS;
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


    setupCgroups(container, 0, 0, 20);
    printf("child created with PID: %ld\n", (long) container);

    close(childProc.pipe_fd[1]);

    if(waitpid(container, NULL, 0) == -1) die("waitpid");

    printf("terminating\n");

    exit(EXIT_SUCCESS);

}
