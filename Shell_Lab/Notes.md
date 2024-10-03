# Eval Function
### Parsing command line
```
int bg = parseline(cmdline, argv); 
  if (argv[0] == NULL)  
    return;   /* ignore empty lines */
```
* The function starts off by parsing the command line into the 'argv' array and determines if the command should run in the background

### Checking for built-in Commands
```
 if (!builtin_cmd(argv)) {
```
* The builtin_cmd function checks if the command is a built-in shell command (quit, job, bg, fg). If it is, the function executes it immediately and returns.

### Blocking Signals
```
sigprocmask(SIG_BLOCK, &mask, &prevmask);
```
* Before forking a new process, the shell blocks SIGCHLD signals to avoid race conditions. The above line blocks the signal and saves the old signal mask. 

### Forking a new process:
```
if ((pid = fork()) < 0) {
    printf("fork(): Error with forking\n");
    return;
}

```
* Fork creates a new child process. If 'fork' fails, an error is printed.
### Child Process execution
```
if (pid == 0) { 
    setpgid(0, 0); 
    if (execvp(argv[0], argv) < 0) {
        printf("%s: Command not found \n", argv[0]);
        exit(0);
    }
}
```
* In the child process, the process group ID is set to the child's PID (using setpgid(0, 0)). This ensures that all processes in the same job share the same process group.
* The child process then attempts to execute the command using 'execvp' and if it fails, an error message is printed and we exit.

### Parent Process Handeling
```
if (bg) {
    addjob(jobs, pid, BG, cmdline);
    sigprocmask(SIG_SETMASK, &prevmask, NULL);
    printf("[%d] (%d) %s", pid2jid(pid), pid, cmdline);
} else {
    addjob(jobs, pid, FG, cmdline);
    sigprocmask(SIG_SETMASK, &prevmask, NULL);
    waitfg(pid);
}
```
* In the parent process, the shell adds the new job to the job list using addjob.
* If the job if a bg (background) job, the shell unblocks the signals and prints a message that indicates the job ID and PID
* If the job is a fg (foreground) job, the shell unblocks the signals and wait for the fg job to complete with waitfg

# Signaling

## Blocking Signals

### Create Signal Set
* A signal set is created and initialized to be empty using the following:
```
sigemptyset(&mask);
```
* Specific signals are added to this set using 'sigaddset'. Here, we are adding SIGCHLD:
```
sigaddset(&mask, SIGCHLD);
```
* Block the signal

### Blocking Signal
```
sigprocmask(SIG_BLOCK, &mask, &prevmask);
```
* The sigprocmask process is used to block signals in 'mask'
* SIG_BLOCK is the action to block the specified signals. prevmask saves the old signal mask
* This ensures that SIGCHLD signals are not delivered to the parent process while it is forking a new child process. It prevents race conditions that could occur if a child process terminates immediately after being forked.

## Unblocking Signals
```
sigprocmask(SIG_SETMASK, &prevmask, NULL);

```
* After forking a new process and adding it to the job list, the sigprocmask function is used again to restore the previous mask signal. The SIG_SETMASK is the action that sets the signal mask to the saved mask in prevmask
* This allows SIGCHLD signals to be delivered again and ensures the parent process can now properly handle any signals from child processes.

* By blocking and unblocking signals during critical sections, this ensures the signals are handles only when the shell is in a safe state to do so.
