#ifndef _BUILTINS_H
#define _BUILTINS_H

#define BUILTINS_COUNT 11

/* builtin shell commands */
int tsh_cd(char **args);
int tsh_help(char **args);
int tsh_exit(char **args);
int tsh_home(char **args);
int tsh_true(char **args);
int tsh_false(char **args);
int tsh_exec(char **args);
int tsh_status(char **args);
int tsh_user(char **args);
int tsh_euid(char **args);
int tsh_basename(char **args);

extern int tsh_status_code;

char *builtin_str[BUILTINS_COUNT];
char *builtin_desc[BUILTINS_COUNT];
int (*builtin_func[BUILTINS_COUNT]) (char**);

#endif
