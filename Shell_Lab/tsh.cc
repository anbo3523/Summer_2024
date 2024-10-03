// 
// tsh - A tiny shell program with job control
// 
// <Angela Booth anbo3523>
//

using namespace std;

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <string>

#include "globals.h"
#include "jobs.h"
#include "helper-routines.h"

//
// Needed global variable definitions
//

static char prompt[] = "tsh> ";
int verbose = 0;

//
// You need to implement the functions eval, builtin_cmd, do_bgfg,
// waitfg, sigchld_handler, sigstp_handler, sigint_handler
//
// The code below provides the "prototypes" for those functions
// so that earlier code can refer to them. You need to fill in the
// function bodies below.
// 

void eval(char *cmdline);
int builtin_cmd(char **argv);
void do_bgfg(char **argv);
void waitfg(pid_t pid);

void sigchld_handler(int sig);
void sigtstp_handler(int sig);
void sigint_handler(int sig);

//
// main - The shell's main routine 
//
int main(int argc, char **argv) 
{
  int emit_prompt = 1; // emit prompt (default)

  //
  // Redirect stderr to stdout (so that driver will get all output
  // on the pipe connected to stdout)
  //
  dup2(1, 2);

  /* Parse the command line */
  char c;
  while ((c = getopt(argc, argv, "hvp")) != EOF) {
    switch (c) {
    case 'h':             // print help message
      usage();
      break;
    case 'v':             // emit additional diagnostic info
      verbose = 1;
      break;
    case 'p':             // don't print a prompt
      emit_prompt = 0;  // handy for automatic testing
      break;
    default:
      usage();
    }
  }

  //
  // Install the signal handlers
  //

  //
  // These are the ones you will need to implement
  //
  Signal(SIGINT,  sigint_handler);   // ctrl-c
  Signal(SIGTSTP, sigtstp_handler);  // ctrl-z
  Signal(SIGCHLD, sigchld_handler);  // Terminated or stopped child

  //
  // This one provides a clean way to kill the shell
  //
  Signal(SIGQUIT, sigquit_handler); 

  //
  // Initialize the job list
  //
  initjobs(jobs);

  //
  // Execute the shell's read/eval loop
  //
  for(;;) {
    //
    // Read command line
    //
    if (emit_prompt) {
      printf("%s", prompt);
      fflush(stdout);
    }

    char cmdline[MAXLINE];

    if ((fgets(cmdline, MAXLINE, stdin) == NULL) && ferror(stdin)) {
      app_error("fgets error");
    }
    //
    // End of file? (did user type ctrl-d?)
    //
    if (feof(stdin)) {
      fflush(stdout);
      exit(0);
    }

    //
    // Evaluate command line
    //
    eval(cmdline);
    fflush(stdout);
    fflush(stdout);
  } 

  exit(0); //control never reaches here
}
  
/////////////////////////////////////////////////////////////////////////////
//
// eval - Evaluate the command line that the user has just typed in
// 
// If the user has requested a built-in command (quit, jobs, bg or fg)
// then execute it immediately. Otherwise, fork a child process and
// run the job in the context of the child. If the job is running in
// the foreground, wait for it to terminate and then return.  Note:
// each child process must have a unique process group ID so that our
// background children don't receive SIGINT (SIGTSTP) from the kernel
// when we type ctrl-c (ctrl-z) at the keyboard.
//
sigset_t mask, prevmask; //Masks for signal blocking
void eval(char *cmdline) 
{
  /* Parse command line */
  //
  // The 'argv' vector is filled in by the parseline
  // routine below. It provides the arguments needed
  // for the execve() routine, which you'll need to
  // use below to launch a process.
  //
  char *argv[MAXARGS];
  pid_t pid; //The current process ID from fork
  sigemptyset(&mask);
  sigaddset(&mask, SIGCHLD);

  int bg = parseline(cmdline, argv); 
  if (argv[0] == NULL)  
    return;   /* ignore empty lines */

  if (!builtin_cmd(argv)) { // checks if the command entered is built-in shell command. If not, the code proceeds to execute command as separate process
    sigprocmask(SIG_BLOCK, &mask, &prevmask); // temporarily blocks certain signals from being delivered to the parent process. This is done to avoid race conditions while the parent process is handling the creation of the child process

    // pid == fork() creates a new process. 
    // If the fork fails it will be less than 0 and we will print an error message.
    if ((pid = fork()) < 0) {
      printf("fork(): Error with forking\n");
      return;
    }

    if (pid == 0) { // checks if the current process is the child process.
      setpgid(0, 0); // if so, sets the process group ID of the process to its own process ID. This helps in managing the process group for job control.
      
      // replaces the child process's image with a new program specified by argv[0] and arguments argv. 
      // If this fails (returns less than 0), then an error is printed and we exit
      if (execvp(argv[0], argv) < 0) {
        printf("%s: Command not found \n", argv[0]);
        exit(0);
      }
    } else {
      if (bg) {
        addjob(jobs, pid, BG, cmdline); // If bg, add job as BG
        sigprocmask(SIG_SETMASK, &prevmask, NULL);
        printf("[%d] (%d) %s", pid2jid(pid), pid, cmdline);
      } else {
        addjob(jobs, pid, FG, cmdline); // If not bg, add job as fg
        sigprocmask(SIG_SETMASK, &prevmask, NULL);
        waitfg(pid);
      }
    }
  }
  return;
}


/////////////////////////////////////////////////////////////////////////////
//
// builtin_cmd - If the user has typed a built-in command then execute
// it immediately. The command name would be in argv[0] and
// is a C string. We've cast this to a C++ string type to simplify
// string comparisons; however, the do_bgfg routine will need 
// to use the argv array as well to look for a job number.
//
int builtin_cmd(char **argv) 
{
  string cmd(argv[0]);

  //Quit Command
  //If the command is quit, the shell needs to exit
  //However, before exiting, the shell loops through all jobs
  //If a job is running, the shell sends SIGINT signal to terminate the job
  //The "-" sign in front of the "-jobs[i].pid, SIGINT" ensures the signal is sent to the entire process group
  //We finally exit the shell program.
  if(cmd == "quit"){
    for (int i = 0; i < MAXJOBS; i++){
      if(jobs[i].pid != 0){
        kill(-jobs[i].pid, SIGINT);
      }
    }
    exit(0);
  }
  //If the command if bg or fg, we call function do_bgfg
  //This is so we can switch the specified process to a background (bg) or foreground (fg)
  else if (cmd == "bg" || cmd == "fg") { //Switching bg/fg
  do_bgfg(argv);
  return 1;
  }
  else if (cmd == "jobs") { // Handle the jobs command
        listjobs(jobs);
        return 1;
  }
  //If the command is &, it is ignored.
  else if (cmd == "&") {
    return 1;
  }
  return 0;     /* not a builtin command */
}

/////////////////////////////////////////////////////////////////////////////
//
// do_bgfg - Execute the builtin bg and fg commands
//
void do_bgfg(char **argv) 
{
  struct job_t *jobp=NULL;
    
  /* Ignore command if no argument */
  if (argv[1] == NULL) {
    printf("%s command requires PID or %%jobid argument\n", argv[0]);
    return;
  }
    
  /* Parse the required PID or %JID arg */
  if (isdigit(argv[1][0])) {
    pid_t pid = atoi(argv[1]);
    if (!(jobp = getjobpid(jobs, pid))) {
      printf("(%d): No such process\n", pid);
      return;
    }
  }
  else if (argv[1][0] == '%') {
    int jid = atoi(&argv[1][1]);
    if (!(jobp = getjobjid(jobs, jid))) {
      printf("%s: No such job\n", argv[1]);
      return;
    }
  }	    
  else {
    printf("%s: argument must be a PID or %%jobid\n", argv[0]);
    return;
  }

  //
  // You need to complete rest. At this point,
  // the variable 'jobp' is the job pointer
  // for the job ID specified as an argument.
  //
  // Your actions will depend on the specified command
  // so we've converted argv[0] to a string (cmd) for
  // your benefit.
  
  //We are sending the SIGCONT signal to the job's process group to continue its execution if it was stopped
  //If the command is fg, the job's state is changed to FG and waitfg is called to wait for the job to complete
  //If the command is bg, the job state is changed to background BG and a message is printed with job ID, PID, and command line.
  string cmd(argv[0]);
  kill(-jobp -> pid, SIGCONT);
  if(cmd == "fg"){
    jobp -> state = FG;
    waitfg(jobp -> pid);
  }
  else{
    jobp -> state = BG;
    printf("[%d] (%d) %s", pid2jid(jobp -> pid), jobp -> pid, jobp -> cmdline);
  }

  return;
}

/////////////////////////////////////////////////////////////////////////////
//
// waitfg - Block until process pid is no longer the foreground process
//

//this function makes the shell wait until the foreground job completes before continuing to other commands
//sleep pauses the execution of the loop for 1 second during each iteration.
//this prevents the loop from continously checking the job state which wastes CPU resources
void waitfg(pid_t pid)
{
  while (fgpid(jobs) == pid){
    sleep(1); // Pausing execution of the loop for 1 second during each iteration
  }
  
  return;
}

/////////////////////////////////////////////////////////////////////////////
//
// Signal handlers
//


/////////////////////////////////////////////////////////////////////////////
//
// sigchld_handler - The kernel sends a SIGCHLD to the shell whenever
//     a child job terminates (becomes a zombie), or stops because it
//     received a SIGSTOP or SIGTSTP signal. The handler reaps all
//     available zombie children, but doesn't wait for any other
//     currently running children to terminate.  
//
void sigchld_handler(int sig) 
{
int olderrno = errno;
    pid_t pid;
    int status;

    while ((pid = waitpid(-1, &status, WNOHANG | WUNTRACED)) > 0) {
        if (WIFEXITED(status)) {
            // Child exited normally
            deletejob(jobs, pid);
        } else if (WIFSIGNALED(status)) {
            // Child was terminated by a signal
            printf("Job [%d] (%d) terminated by signal %d\n", pid2jid(pid), pid, WTERMSIG(status));
            deletejob(jobs, pid);
        } else if (WIFSTOPPED(status)) {
            // Child was stopped by a signal
            struct job_t *job = getjobpid(jobs, pid);
            if (job != NULL) {
                job->state = ST;
                printf("Job [%d] (%d) stopped by signal %d\n", pid2jid(pid), pid, WSTOPSIG(status));
            }
        }
    }

    errno = olderrno;
}

/////////////////////////////////////////////////////////////////////////////
//
// sigint_handler - The kernel sends a SIGINT to the shell whenver the
//    user types ctrl-c at the keyboard.  Catch it and send it along
//    to the foreground job.  
//
void sigint_handler(int sig) 
{
pid_t pid = fgpid(jobs);
    if (pid != 0) {
        kill(-pid, SIGINT); // Send SIGINT to the process group
    }
}

/////////////////////////////////////////////////////////////////////////////
//
// sigtstp_handler - The kernel sends a SIGTSTP to the shell whenever
//     the user types ctrl-z at the keyboard. Catch it and suspend the
//     foreground job by sending it a SIGTSTP.  
//
void sigtstp_handler(int sig) 
{
   pid_t pid = fgpid(jobs);
    if (pid != 0) {
        kill(-pid, SIGTSTP); // Send SIGTSTP to the process group
    }
}

/*********************
 * End signal handlers
 *********************/




