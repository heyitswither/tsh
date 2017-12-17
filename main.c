#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <pwd.h>
#include <stdbool.h>

#define TSH_RL_BUFSIZE 1024
#define TSH_TOK_BUFSIZE 64
#define TSH_TOK_DELIM " \t\r\n\a"

int tsh_loop(void);
char* tsh_read_line(void);
char** tsh_split_line(char* line);
int tsh_launch(char** args);
int tsh_num_builtins(void);
int tsh_execute(char** args);
char* tsh_get_home(void);
char* tsh_get_pwd(void);

/* builtin shell commands */
int tsh_cd(char** args);
int tsh_help(char** args);
int tsh_exit(char** args);
int tsh_pwd(char** args);
int tsh_home(char** args);
int tsh_true(char** args);
int tsh_false(char** args);
int tsh_exec(char** args);

char *builtin_str[] = {
  "cd",
  "help",
  "exit",
  "pwd",
  "home",
  "true",
  "false",
  "exec"
};

int (*builtin_func[]) (char **) = {
  &tsh_cd,
  &tsh_help,
  &tsh_exit,
  &tsh_pwd,
  &tsh_home
};

int tsh_num_builtins(void) {
  return sizeof(builtin_str) / sizeof(char *);
}

int main(/*int argc, char** argv*/ void) {
    // load configuration files

    // main loop
    int success = tsh_loop();

    // exit cleanly, freeing everything
    return success;
}

int tsh_execute(char **args)
{
  int i;

  if (args[0] == NULL) {
    // An empty command was entered.
    return 1;
  }

  for (i = 0; i < tsh_num_builtins(); i++) {
    if (strcmp(args[0], builtin_str[i]) == 0) {
      return (*builtin_func[i])(args);
    }
  }

  return tsh_launch(args);
}

int tsh_pwd(char** args)
{
    char* cwd = malloc(TSH_RL_BUFSIZE);
    cwd = tsh_get_pwd();
    printf("%s\n", cwd);
    return 1;
}

int tsh_home(char** args)
{
    struct passwd *pw = getpwuid(getuid());
    const char *homedir = pw->pw_dir;
    printf("%s\n", homedir);
    return 1;
}

int tsh_exit(char **args)
{
  return 0;
}

int tsh_true(char** args)
{
    return true;
}

int tsh_false(char** args)
{
    return false;
}

int tsh_exec(char** args)
{
    execvp(args[0], args);
    return 1;
}

int tsh_help(char **args)
{
  int i;
  printf("Tucker Boniface's TSH\n");
  printf("A fork of Stephen Brennan's LSH\n");
  printf("The following are built in:\n");

  for (i = 0; i < tsh_num_builtins(); i++) {
    printf("  %s\n", builtin_str[i]);
  }

  printf("Use the man command for information on other programs.\n");
  return 1;
}

int tsh_cd(char **args)
{
  if (args[1] == NULL) {
    fprintf(stderr, "tsh: expected argument to \"cd\"\n");
  } else {
    if (chdir(args[1]) != 0) {
      perror("tsh");
    }
  }
  return 1;
}

char* tsh_get_pwd(void) {
    char* cwd = malloc(TSH_RL_BUFSIZE);
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("tsh");
        return "";
    }
    return cwd;
}

int tsh_launch(char **args)
{
    pid_t pid, wpid;
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
            wpid = waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return 1;
}

char** tsh_split_line(char *line)
{
    int bufsize = TSH_TOK_BUFSIZE, position = 0;
    char **tokens = malloc(bufsize * sizeof(char*));
    char *token;

    if (!tokens) {
        fprintf(stderr, "tsh: allocation error\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, TSH_TOK_DELIM);
    while (token != NULL) {
        tokens[position] = token;
        position++;

        if (position >= bufsize) {
            bufsize += TSH_TOK_BUFSIZE;
            tokens = realloc(tokens, bufsize * sizeof(char*));
            if (!tokens) {
                fprintf(stderr, "tsh: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }

        token = strtok(NULL, TSH_TOK_DELIM);
    }
    tokens[position] = NULL;
    return tokens;
}

char* tsh_read_line(void)
{
    int bufsize = TSH_RL_BUFSIZE;
    int position = 0;
    char *buffer = malloc(sizeof(char) * bufsize);
    int c;

    if (!buffer) {
        fprintf(stderr, "tsh: allocation error\n");
        exit(EXIT_FAILURE);
    }

    while (1) {
        // Read a character
        c = getchar();

        // If we hit EOF, replace it with a null character and return.
        if (c == EOF) {
            printf("exit\n");
            exit(0);
        } else if (c == '\n') {
            buffer[position] = '\0';
            return buffer;
        } else {
            buffer[position] = c;
        }
        position++;

        // If we have exceeded the buffer, reallocate.
        if (position >= bufsize) {
            bufsize += TSH_RL_BUFSIZE;
            buffer = realloc(buffer, bufsize);
            if (!buffer) {
                fprintf(stderr, "tsh: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
    }
}

int tsh_loop(void) {
    char *line;
    char **args;
    int status;
    do {
        printf("> ");
        line = tsh_read_line();
        args = tsh_split_line(line);
        status = tsh_execute(args);
        free(line);
        free(args);
    } while (status);
    return EXIT_SUCCESS;
}
