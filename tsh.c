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
// user hostname directory $/#
//#define TSH_PROMPT "%s@%s:%s%s "
#define TSH_PROMPT "[%s@%s %s]%s "

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
int tsh_get_euid(void);
char* tsh_get_hostname(void);
void tsh_clean_exit(void);
char* tsh_get_basename(char* path);

/* builtin shell commands */
int tsh_cd(char** args);
int tsh_help(char** args);
int tsh_exit(char** args);
int tsh_home(char** args);
int tsh_true(char** args);
int tsh_false(char** args);
int tsh_exec(char** args);
int tsh_status(char** args);
int tsh_user(char** args);
int tsh_euid(char** args);
int tsh_basename(char** args);

char* builtin_str[] = {
  "cd", "help", "exit", "home", "true", "false", "exec", "status", "user", "euid", "basename"
};
char* builtin_desc[] = {
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

int tsh_euid(char** args)
{
    int euid = tsh_get_euid();
    printf("%d\n", euid);
    return 0;
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

int tsh_home(char** args)
{
    char* home = malloc(sizeof(char)*TSH_RL_BUFSIZE);
    home = tsh_get_home();
    printf("%s\n", home);
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

int tsh_basename(char** args)
{
    int i = 1;
    while (args[i] != NULL) {
        printf("%s\n", tsh_get_basename(args[i]));
        i++;
    }
    return 0;
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

char* tsh_get_home(void)
{
    struct passwd* pw = getpwuid(getuid());
    char* homedir = pw->pw_dir; 
    return homedir;
}

char* tsh_get_hostname(void) {
    char* hostname = malloc(sizeof(char)*TSH_RL_BUFSIZE);
    gethostname(hostname, sizeof(char)*TSH_RL_BUFSIZE);
    return hostname;
}

char* tsh_get_basename(char* path) {
    char* base = strrchr(path, '/');
    return base ? base+1 : path;
}

int tsh_get_euid(void) {
    int euid = 0;
    euid = geteuid();
    return euid;
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

char *str_replace(char *orig, char *rep, char *with) {
    char *result; // the return string
    char *ins;    // the next insert point
    char *tmp = NULL;    // varies
    int len_rep;  // length of rep (the string to remove)
    int len_with; // length of with (the string to replace rep with)
    int len_front; // distance between rep and end of last rep
    int count;    // number of replacements

    // sanity checks and initialization
    if (!orig || !rep)
        return NULL;
    len_rep = strlen(rep);
    if (len_rep == 0)
        return NULL; // empty rep causes infinite loop during count
    if (!with)
        with = "";
    len_with = strlen(with);

    // count the number of replacements needed
    ins = orig;
    for (count = 0; tmp == strstr(ins, rep); ++count) {
        ins = tmp + len_rep;
    }

    tmp = result = malloc(strlen(orig) + (len_with - len_rep) * count + 1);

    if (!result)
        return NULL;

    while (count--) {
        ins = strstr(orig, rep);
        len_front = ins - orig;
        tmp = strncpy(tmp, orig, len_front) + len_front;
        tmp = strcpy(tmp, with) + len_with;
        orig += len_front + len_rep; // move to next "end of rep"
    }
    strcpy(tmp, orig);
    return result;
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
