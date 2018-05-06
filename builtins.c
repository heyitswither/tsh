#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "lib/libtsh.h"
#include "lib/builtins.h"

int tsh_status_code = 0;

char *builtin_str[] = {
    "cd", "help", "exit", "home", "true", "false", "exec", "status", "user", "euid", "basename"
};

char *builtin_desc[] = {
    "changes the current directory",
    "shows this message",
    "exits the shell",
    "prints the path to home",
    "returns true",
    "returns false",
    "replaces the current process",
    "prints or changes the status",
    "prints the current logged in user",
    "prints the EUID of the current user",
    "prints path with any leading directory components removed"
};

int (*builtin_func[]) (char**) = {
    &tsh_cd,
    &tsh_help,
    &tsh_exit,
    &tsh_home,
    &tsh_true,
    &tsh_false,
    &tsh_exec,
    &tsh_status,
    &tsh_user,
    &tsh_euid,
    &tsh_basename
};

int tsh_euid(char **args)
{
    int euid = tsh_get_euid();
    printf("%d\n", euid);
    return 0;
}

int tsh_user(char **args)
{
    char *user = malloc(sizeof(char)*TSH_RL_BUFSIZE);
    user = tsh_get_user();
    printf("%s\n", user);
    return 0;
}

int tsh_status(char **args)
{
    if (args[1] == NULL) {
        printf("%d\n", tsh_status_code);
    } else {
        sscanf(args[1], "%d", &tsh_status_code);
        return tsh_status_code;
    }
    return 0;
}

int tsh_home(char **args)
{
    char *home = malloc(sizeof(char)*TSH_RL_BUFSIZE);
    home = tsh_get_home();
    printf("%s\n", home);
    return 0;
}

int tsh_exit(char **args)
{
    return -1;
}

int tsh_true(char **args)
{
    return 0;
}

int tsh_false(char **args)
{
    return 1;
}

int tsh_exec(char **args)
{
    if (execvp(args[0], args) == -1) {
        perror("tsh");
    }    
    return 1;
}

int tsh_basename(char **args)
{
    for (int i = 1; args[i] != NULL; i++) {
        printf("%s\n", tsh_get_basename(args[i]));
    }
    return 0;
}

int tsh_help(char **args)
{
    printf("Tucker Boniface's TSH\n");
    printf("A fork of Stephen Brennan's LSH\n");
    printf("The following are built in:\n");

    for (int i = 0; i < BUILTINS_COUNT; i++) {
        printf("  %s - %s\n", builtin_str[i], builtin_desc[i]);
    }

    printf("Use the man command for information on other programs.\n");
    return 0;
}

int tsh_cd(char **args)
{
    if (args[1] == NULL) {
        if (chdir(tsh_get_home()) != 0) {
            perror("tsh");
        }
    } else {
        if (chdir(args[1]) != 0) {
            perror("tsh");
        }
    }
    return 0;
}

