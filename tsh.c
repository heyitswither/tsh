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

int tsh_status_code = 0;

char** tsh_split_line(char* line);
int tsh_launch(char** args);
int tsh_execute(char** args);
int tsh_loop(void);
char* tsh_read_line(void);
int tsh_num_builtins(void);
char* tsh_get_home(void);
char* tsh_get_pwd(void);
char* tsh_get_user(void);
void tsh_clean_exit(void);

/* builtin shell commands */
int tsh_cd(char** args);
int tsh_help(char** args);
int tsh_exit(char** args);
int tsh_pwd(char** args);
int tsh_home(char** args);
int tsh_true(char** args);
int tsh_false(char** args);
int tsh_exec(char** args);
int tsh_status(char** args);
int tsh_user(char** args);

char* builtin_str[] = {
  "cd", "help", "exit", "pwd", "home", "true", "false", "exec", "status", "user"
};
char* builtin_desc[] = {
  "changes the current directory",
  "shows this message",
  "exits the shell",
  "prints the current directory",
  "prints the path to home",
  "returns true",
  "returns false",
  "replaces the current process",
  "prints or changes the status",
  "prints the current logged in user"
};

int (*builtin_func[]) (char**) = {
  &tsh_cd,
  &tsh_help,
  &tsh_exit,
  &tsh_pwd,
  &tsh_home,
  &tsh_true,
  &tsh_false,
  &tsh_exec,
  &tsh_status,
  &tsh_user
};

int tsh_num_builtins(void) {
  return sizeof(builtin_str) / sizeof(char*);
}

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

int tsh_user(char** args)
{
    char* user = malloc(sizeof(char)*TSH_RL_BUFSIZE);
    user = tsh_get_user();
    printf("%s\n", user);
    return 0;
}

int tsh_status(char** args)
{
    if (args[1] == NULL) {
        printf("%d\n", tsh_status_code);
    } else {
        int new_code = 0;
        sscanf(args[1], "%d", &new_code);
        tsh_status_code = new_code;
        printf("Status set to %d\n", new_code);
        return new_code;
    }
    return 0;
}

int tsh_pwd(char** args)
{
    char* cwd = malloc(TSH_RL_BUFSIZE);
    cwd = tsh_get_pwd();
    printf("%s\n", cwd);
    return 0;
}

int tsh_home(char** args)
{
    struct passwd* pw = getpwuid(getuid());
    const char* homedir = pw->pw_dir;
    printf("%s\n", homedir);
    return 0;
}

int tsh_exit(char** args)
{
  return -1;
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
    if (execvp(args[0], args) == -1) {
        perror("tsh");
    }    
    return 1;
}

int tsh_help(char** args)
{
  int i;
  printf("Tucker Boniface's TSH\n");
  printf("A fork of Stephen Brennan's LSH\n");
  printf("The following are built in:\n");

  for (i = 0; i < tsh_num_builtins(); i++) {
    printf("  %s - %s\n", builtin_str[i], builtin_desc[i]);
  }

  printf("Use the man command for information on other programs.\n");
  return 0;
}

int tsh_cd(char** args)
{
  if (args[1] == NULL) {
    fprintf(stderr, "tsh: expected argument to \"cd\"\n");
  } else {
    if (chdir(args[1]) != 0) {
      perror("tsh");
    }
  }
  return 0;
}

char* tsh_get_pwd(void) {
    char* cwd = malloc(sizeof(char)*TSH_RL_BUFSIZE);
    if (getcwd(cwd, sizeof(char)*TSH_RL_BUFSIZE) == NULL) {
        perror("tsh");
        return "";
    }
    return cwd;
}

char* tsh_get_user(void)
{
    char* user = malloc(sizeof(char)*TSH_RL_BUFSIZE);
    user = getlogin();
    return user;
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

char** tsh_split_line(char *line)
{
    int bufsize = TSH_TOK_BUFSIZE, position = 0;
    char** tokens = malloc(bufsize * sizeof(char*));
    char* token;

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
    char* buffer = malloc(sizeof(char) * bufsize);
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
    char* line;
    char** args;
    do {
        printf("> ");
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
