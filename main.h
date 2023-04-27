#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <limits.h>
#include <pwd.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include <time.h>
#include <signal.h>
#include <sys/stat.h>

#include "general.h"

#define REDIRECTION1 "<"
#define REDIRECTION2 ">"
#define BLANK_STR " "
#define FILE_END_CHAR '\0'
#define EXIT_COMMAND ":q\n"
#define CD_COMMAND "cd "
#define HISTORY_COMMAND "history"
#define PIPE_CHAR "|"

#define FLAGS1 O_WRONLY | O_CREAT | O_TRUNC
#define FLAGS2 S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH

#define PIPE_READ_ONLY 0
#define PIPE_WRITE_ONLY 1
#define MAX_PROCESSES 1024
#define MAX_HISTORY 4096
#define MAX_STR_LEN 4096

pid_t childs[MAX_PROCESSES];
char history[MAX_HISTORY][MAX_STR_LEN];
char buff[MAX_STR_LEN];
char *will_exec[MAX_STR_LEN];
int child_count = 0;
int history_count = 0;
char *log_filename;


void two_dim_arr_free(char **buffer) {
    for (int i = 0; buffer[i] != NULL; i++) free(buffer[i]);
}

void history_append(const char *cmd_str) {
    int i;
    for (i = 0; cmd_str[i] != FILE_END_CHAR && i < MAX_STR_LEN - 1 ; i++) history[history_count][i] = cmd_str[i];
    
    history[history_count][i] = FILE_END_CHAR;
    history_count++;
}


void show_history() {
    for (int i = 0; i < history_count; i++) printf("%d %s\n", i + 1, history[i]);
}

void print_prompt_string() {
    char host[MAX_STR_LEN],str[MAX_STR_LEN];
    struct passwd *uid = getpwuid(getuid()); //getting uid
    if (gethostname(host, sizeof(host)) < 0) error_exit("host get");
    if (getcwd(str, sizeof(str)) == NULL) error_exit("getcwd");
    printf("%s@%s:%s$ ", uid->pw_name, host, str);
}

char *show_timestamp() {
    time_t time0 = time(NULL);
    char *time_str = ctime(&time0);
    time_str[strlen(time_str) - 1] = FILE_END_CHAR;
    return time_str;
}

char *create_log_name() {
    static char str[MAX_STR_LEN];
    snprintf(str, MAX_STR_LEN, "log_%s.log", show_timestamp());
    for (int i = 0; str[i] != FILE_END_CHAR; i++) str[i] =  str[i] == ' ' || str[i] == ':' ? '_' : str[i];
    return str;
}

void log_child_info(const char *cmd_str, pid_t child_pid) {
    FILE *fd = fopen(log_filename, "a");
    if (!fd) error_exit("logging file open");
    fprintf(fd, "CHILD PID: %d -> Command: [%s]\n", child_pid, cmd_str);
    fclose(fd);
}

void sig_sigint(int sig) {
    for (int i = 0; i < child_count; i++) {
        kill(childs[i], SIGTERM);
        printf("\nTerminated child pid: %d \n", childs[i]);
    }
    
    child_count = 0;
    printf("\nPID: %d -> ALL CHILD PROCESSES ARE TERMINATED...\n", getpid());
}

void sig_handler(int sig) {
    if (sig == SIGABRT) {
        perror("\nReceived SIGABRT.\n");
    } else if (sig == SIGALRM) {
        perror("\nReceived SIGALRM.\n");
    } else if (sig == SIGBUS) {
        perror("\nReceived SIGBUS.\n");
    } else if (sig == SIGCHLD) {
        perror("\nReceived SIGCHLD.\n");
    } else if (sig == SIGCONT) {
        perror("\nReceived SIGCONT.\n");
    } else if (sig == SIGFPE) {
        perror("\nReceived SIGFPE.\n");
    } else if (sig == SIGHUP) {
        perror("\nReceived SIGHUP.\n");
    } else if (sig == SIGILL) {
        perror("\nReceived SIGILL.\n");
    } else if (sig == SIGINT) {
        perror("\nReceived SIGINT.\n");
        sig_sigint(sig);
    } else if (sig == SIGKILL) {
        perror("\nReceived SIGKILL.\n");
        sig_sigint(sig);
    } else if (sig == SIGPIPE) {
        perror("\nReceived SIGPIPE.\n");
    } else if (sig == SIGQUIT) {
        perror("\nReceived SIGQUIT.\n");
    } else if (sig == SIGSEGV) {
        perror("\nReceived SIGSEGV.\n");
    } else if (sig == SIGSTOP) {
        perror("\nReceived SIGSTOP.\n");
    } else if (sig == SIGTERM) {
        perror("\nReceived SIGTERM.\n");
        sig_sigint(sig);
    } else if (sig == SIGTSTP) {
        perror("\nReceived SIGTSTP.\n");
    } else if (sig == SIGTTIN) {
        perror("\nReceived SIGTTIN.\n");
    } else if (sig == SIGTTOU) {
        perror("\nReceived SIGTTOU.\n");
    } else if (sig == SIGUSR1) {
        perror("\nReceived SIGUSR1.\n");
    } else if (sig == SIGUSR2) {
        perror("\nReceived SIGUSR2.\n");
    } else if (sig == SIGPOLL) {
        perror("\nReceived SIGPOLL.\n");
    } else if (sig == SIGPROF) {
        perror("\nReceived SIGPROF.\n");
    } else if (sig == SIGSYS) {
        perror("\nReceived SIGSYS.\n");
    } else if (sig == SIGTRAP) {
        perror("\nReceived SIGTRAP.\n");
    } else if (sig == SIGURG) {
        perror("\nReceived SIGURG.\n");
    } else if (sig == SIGVTALRM) {
        perror("\nReceived SIGVTALRM.\n");
    } else if (sig == SIGXCPU) {
        perror("\nReceived SIGXCPU.\n");
    } else if (sig == SIGXFSZ) {
        perror("\nReceived SIGXFSZ.\n");
    } else {
        perror("Unknown sig\n");
    }
}

void start_sigaction() {
    struct sigaction sig;
    sig.sa_handler = sig_handler;
    sigaddset(&sig.sa_mask, SIGINT);
    sigemptyset(&sig.sa_mask);
    sig.sa_flags = 0;

    if (sigaction(SIGINT, &sig, NULL) == -1) error_exit("sigaction");
    
}