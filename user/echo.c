#include "lib.h"

void
umain(int argc, char **argv)
{
    int i, nflag;

    nflag = 0;
    if (argc > 1 && strcmp(argv[1], "-n") == 0) {
        nflag = 1;
        argc--;
        argv++;
    }   
    for (i = 1; i < argc; i++) {
        if (i > 1)
            write(1, " ", 1);
        if (argv[i][0] == '$') {
            char value[128];
            syscall_env_var(&argv[i][1], value, 0, 0, 1);
            write(1, value, strlen(value));
        } else write(1, argv[i], strlen(argv[i]));
    }   
    if (!nflag)
        write(1, "\n", 1); 
}