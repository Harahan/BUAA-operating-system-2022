#include "lib.h"
#include "sh.h"
#include "../include/error.h"
int flag[256];
void unset(char *name) {
    int r;
    if ((r = syscall_env_var(name, "", 0, 0, 3)) < 0) {
        if (r == -E_ENV_VAR_NOT_FOUND) fwritef(1, "environment var " RED([%s]) "doesn't exists!\n", name);
        else fwritef(1, "environment var" RED([%s]) "is readonly!\n", name);
    }
}
void usage(void) {
    fwritef(1, "usage: unset [vars...]\n");
    exit();
}

void umain(int argc, char **argv) {
    int i;
    ARGBEGIN
    {
        default: usage();
        case 'd':
        case 'F':
        case 'l':
            flag[(u_char) ARGC()]++;
    }
    ARGEND
    if (argc == 0) return;
    else for (i = 0; i < argc; i++) unset(argv[i]);
}