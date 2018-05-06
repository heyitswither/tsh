#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pwd.h>

#include "lib/libtsh.h"

#define _LIBTSH_C

char *tsh_get_home(void)
{
    struct passwd* pw = getpwuid(getuid());
    char* homedir = pw->pw_dir; 
    return homedir;
}

char *tsh_get_hostname(void) {
    char *hostname = malloc(sizeof(char)*TSH_RL_BUFSIZE);
    gethostname(hostname, sizeof(char)*TSH_RL_BUFSIZE);
    return hostname;
}

char *tsh_get_basename(char* path) {
    char *base = strrchr(path, '/');
    return base ? base+1 : path;
}

int tsh_get_euid(void) {
    return geteuid();
}

char* tsh_get_pwd(void) {
    char *cwd = malloc(sizeof(char)*TSH_RL_BUFSIZE);
    if (getcwd(cwd, sizeof(char)*TSH_RL_BUFSIZE) == NULL) {
        perror("tsh");
        return "";
    }
    return cwd;
}

char *tsh_get_user(void)
{
    char *user = malloc(sizeof(char)*TSH_RL_BUFSIZE);
    user = getlogin();
    return user;
}

char **tsh_split_line(char* line)
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

char *tsh_read_line(void)
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
