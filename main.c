#include "main.h"


void parser(const char *cmd_str, char **buffer) {
    char *tok;
    int count = 0;

    char *tmp = strdup(cmd_str);
    tok = strtok(tmp, BLANK_STR);
    while (tok != NULL) {
        buffer[count++] = strdup(tok);
        tok = strtok(NULL, BLANK_STR);
    }
    buffer[count] = NULL;

    free(tmp);
}

void redirection_start(char **buffer) {

    for (int i = 0; buffer[i] != NULL; i++) {
        if (my_strcmp(buffer[i], REDIRECTION1) == 0) {
            int fd = open(buffer[i + 1], O_RDONLY);
            if (fd < 0) error_exit("file");

            dup2(fd, 0);
            buffer[i] = NULL;
            close(fd);
        } else if (my_strcmp(buffer[i], REDIRECTION2) == 0) {
            int fd2 = open(buffer[i + 1], FLAGS1, FLAGS2);
            if (fd2 < 0) error_exit("file");

            dup2(fd2, 1);
            buffer[i] = NULL;
            close(fd2);
        }
    }
}

pid_t command_start(const char *cmd_str, int fd, int fd2) {
    char *buffer[MAX_STR_LEN];
    parser(cmd_str, buffer);

    pid_t pid = fork();
    if (pid == 0) {
        if (fd != 0) {
            dup2(fd, 0);
            close(fd);
        }
        if (fd2 != 1) {
            dup2(fd2, 1);
            close(fd2);
        }
        redirection_start(buffer);
        execvp(buffer[0], buffer);
        perror("execvp failed");
        for (int i = 0; i < child_count; i++) free(buffer[i]);
        
        
    }else {
        log_child_info(cmd_str, pid);
        childs[child_count++] = pid;
    }

    two_dim_arr_free(buffer);

    return pid;
}

void pipeline_start(char *will_exec[], int num) {
    int fd = 0, fd_pipe[2];

    for (int i = 0; i < num - 1; i++) {
        if (pipe(fd_pipe) < 0) error_exit("pipe");

        command_start(will_exec[i], fd, fd_pipe[PIPE_WRITE_ONLY]);
        close(fd_pipe[PIPE_WRITE_ONLY]);

        fd = fd_pipe[PIPE_READ_ONLY];
    }

    command_start(will_exec[num - 1], fd, 1);

    if (fd != 0) close(fd);
    
    for (int i = 0; i < num; i++) wait(NULL);

    child_count = 0;

}


int tokanizer(){
    int num;
    char *key;
    num = 0;
    key = strtok(buff, PIPE_CHAR);
    while (key != NULL) {
        will_exec[num++] = strdup(key);
        key = strtok(NULL, PIPE_CHAR);
    }

    return num;
}

void code(){
    int num;
    start_sigaction();

    while (1) {
        print_prompt_string();

        if(fgets(buff, MAX_STR_LEN, stdin) == NULL){
            continue;
        }

        if (my_strcmp(buff, EXIT_COMMAND) == 0){
            printf("\n\nPROGRAM TERMINATED \n\n");
            for (int i = 0; i < child_count; i++) {
                kill(childs[i], SIGTERM);
                printf("\nTerminated child pid: %d \n", childs[i]);
            }
            exit(EXIT_SUCCESS);
        } 

        buff[strlen(buff) - 1] = FILE_END_CHAR;
        history_append(buff);
        
        log_filename = create_log_name();
        num = tokanizer();
        
        if (num > 1) {
            pipeline_start(will_exec, num);
            for (int i = 0; i < num; i++) free(will_exec[i]);

        } else {
            
            if (strncmp(buff, CD_COMMAND, strlen(CD_COMMAND)) == 0) my_chdir(buff + strlen(CD_COMMAND));
            else if (my_strcmp(buff, HISTORY_COMMAND) == 0) show_history();
            else {
                command_start(buff, 0, 1);
                wait(NULL);
                child_count = 0;
            }
        }
        
    }
   
}

int main() {
    code();
    return 0;
}
