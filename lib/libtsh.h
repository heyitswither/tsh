#ifndef _LIBTSH_H
#define _LIBTSH_H

#define TSH_RL_BUFSIZE 1024
#define TSH_TOK_BUFSIZE 64
#define TSH_TOK_DELIM " \t\r\n\a"
// user hostname directory $/#
//#define TSH_PROMPT "%s@%s:%s%s "
#define TSH_PROMPT "[%s@%s %s]%s "

char *tsh_get_basename(char *path);
char *tsh_get_home(void);
char *tsh_get_pwd(void);
char *tsh_get_user(void);
int tsh_get_euid(void);
char *tsh_get_hostname(void);
char **tsh_split_line(char *line);
char *tsh_read_line(void);

#endif
