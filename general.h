#include <stdio.h>
#include <signal.h>

int my_strlen(const char* str) {
    int length = 0;
    while (*(str + length) != '\0') {
        length++;
    }
    return length;
}

void my_write(const char* str) {
    int length = my_strlen(str);
    write(STDOUT_FILENO, str, length);
}

int my_atoi(const char *str) {
    int rez = 0;
    int is_positive = 1;
    if (*str == '-') {
        is_positive = -1;
        str++;
    }
    while (*str != '\0') {
        if (*str >= '0' && *str <= '9') {
            rez = rez * 10 + (*str - '0');
        } else {
            my_write("atoi: no number not detected\n");
            return 0;
        }
        str++;
    }
    return rez * is_positive;
}

int my_strcmp(const char *str1, const char *str2) {

    for (int i = 0; str1[i] != '\0' || str2[i] != '\0'; i++)
        if (str1[i] != str2[i]) return str1[i] - str2[i];

    return 0;
}

void error_exit(char *str){
    strcat(str, "failed");
    exit(EXIT_FAILURE);
}

void my_chdir(const char *path) {
    if (chdir(path)) error_exit("chdir");
}
