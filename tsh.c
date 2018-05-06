#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "lib/libtsh.h"
#include "lib/builtins.h"

#ifndef _BUILTINS_H
int tsh_status_code = 0;
#endif

int tsh_launch(char** args);
int tsh_execute(char** args);
int tsh_loop(void);
void tsh_clean_exit(void);

int main(/*int argc, char** argv*/ void) {
    // load configuration files

    // main loop
    int success = tsh_loop();

    // exit cleanly, freeing everything
    tsh_clean_exit();   

    return success;
}

void tsh_clean_exit(void)
{
    ;
}

int tsh_execute(char **args)
{
  if (args[0] == NULL) {
    // An empty command was entered.
    return 1;
  }

  #ifdef _BUILTINS_H
  for (int i = 0; i < BUILTINS_COUNT; i++) {
    if (strcmp(args[0], builtin_str[i]) == 0) {
      return (*builtin_func[i])(args);
    }
  }
  #endif

  return tsh_launch(args);
}

int tsh_launch(char** args)
{
    pid_t pid;
    int status;

    pid = fork();
    if (pid == 0) {
        // Child process
        if (execvp(args[0], args) == -1) {
            perror("tsh");
        }
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        // Error forking
        perror("tsh");
    } else {
        // Parent process
        do {
            waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return 1;
}

int tsh_loop(void) {
    char* line;
    char** args;
    do {
        printf(TSH_PROMPT, tsh_get_user(), tsh_get_hostname(), tsh_get_basename(tsh_get_pwd()), (tsh_get_euid() == 0 ? "#" : "$"));
        line = tsh_read_line();
        args = tsh_split_line(line);
        tsh_status_code = tsh_execute(args);
        if (tsh_status_code < -1)
            return EXIT_FAILURE;
        free(line);
        free(args);
    } while (tsh_status_code != -1);
    return EXIT_SUCCESS;
}
