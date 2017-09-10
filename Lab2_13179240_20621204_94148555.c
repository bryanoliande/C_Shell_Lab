// Oliande, Bryan: 13179240
// Tryon, Daniel: 20621204
// Ishikawa, Takahiro: 94148555


//LAB 2 SUBMISSION

#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

// NEW
#include <signal.h>

#define MAXARGS 128
#define MAXLINE 256
#define TRUE 1

// Function Prototypes
void eval(char *cmdline);
int parseline(char *buf, char **argv);
int builtin_command(char **argv);
void unix_error(const char *msg);
void handle_sigchld(int sig);

extern char **environ;


int main()
{
    char cmdline[MAXLINE];  // Command line
    
    // install signal handler
    signal(SIGCHLD, handle_sigchld);
    
    while (TRUE) {
        // Read
        printf("> ");
        fgets(cmdline, MAXLINE, stdin);
        if (feof(stdin))
            exit(0);
        
        // Evaluate
        eval(cmdline);
    }
}

// eval - Evaluate a command line
void eval(char *cmdline)
{
    char *argv[MAXARGS];    // Argument list execve()
    char buf[MAXLINE];      // Holds modified command line
    int bg;                 // Should the job run in bg or fg?
    pid_t pid;              // Process id
    
    strcpy(buf, cmdline);
    bg = parseline(buf, argv);
    if (argv[0] == NULL)
        return;     // ignore empty lines
    
    if (!builtin_command(argv)) {
        if ((pid = fork()) == 0) {
            if (execve(argv[0], argv, environ) < 0) {
                if (execvp(argv[0], argv) < 0) {
                    printf("%s: Command not found.\n", argv[0]);
                    exit(0);
                }
            }
        }
        
        // Parent waits for foreground job to terminate
        if (!bg) {
            int status;
            if (waitpid(pid, &status, 0) < 0)
                unix_error("aitfg: waitpid error");
        }
        else
            printf("%d %s", pid, cmdline);
    }
    return;
}

// If first arg is a builtin command, run it and return true
int builtin_command(char **argv)
{
    if (!strcmp(argv[0], "quit"))   // quit command
        exit(0);
    if (!strcmp(argv[0], "&"))      // ignore singleton &
        return 1;
    return 0;                       // not builtin command
}

// parseline - Parse the command line and build the argv array
int parseline(char *buf, char **argv)
{
    char *delim;        // Points to first space delimiter
    int argc;           // Number of args
    int bg;             // Background job?
    
    buf[strlen(buf)-1] = ' ';   // Replace carriage return with space
    while (*buf && ((*buf == ' ')||(*buf == '\t')))   // Ignore leading spaces
        buf++;
    
    // Build argv list
    argc = 0;
    while ((delim = strchr(buf, ' '))) {
        argv[argc++] = buf;
        *delim = '\0';
        buf = delim + 1;
        while (*buf && ((*buf == ' ')||(*buf== '\t')))   // Ignore spaces
            buf++;
    }
    
    argv[argc] = NULL;
    
    if (argc == 0)  // Ignore blank line
        return 1;
    
    int x;
    char *y;
    
    if (*argv[argc-1]) {
        x = strlen(argv[argc-1]);
        y = argv[argc-1];
        y += (x-1);
    }
    
    // Should the job run in the background?
    if ((bg = (*argv[argc-1] == '&')) != 0)
        argv[--argc] = NULL;
    
    else if (*y == '&') {
        *y = '\0';
        bg = 1;
    }
    
    return bg;
}

void unix_error(const char *msg)
{
    int errnum = errno;
    fprintf(stderr, "%s (%d : %s)\n", msg, errnum, strerror(errnum));
    exit(EXIT_FAILURE);
}

void handle_sigchld(int sig) {
    
    
    // DEBUG
    // printf("%d received %d\n", getpid(), sig);
    
    int saved_errno = errno;
    while (waitpid((pid_t)(-1), 0, WNOHANG) > 0) {}
    errno = saved_errno;
}


