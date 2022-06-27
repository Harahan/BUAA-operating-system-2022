#include "lib.h"
#include "sh.h"
int flag[256];
void touch(char *path, char *prefix) {
    int r, fd;
    char curpath[MAXPATHLEN] = {'\0'};
    // if ((r = curpath_get(curpath)) < 0) fwritef(1, "touch: can't get environment var [curpath]\n");
    // absolute path
    if (path[0] == '/') strcpy(curpath, path);
    else {
        if (curpath[strlen(curpath) - 1] != '/') strcat(curpath, "/");
        strcat(curpath, path);
    }
    if ((r = create(curpath, FTYPE_REG)) < 0) {
        fwritef(1, "file %d already exists!\n", curpath);
        return;
    }
    fwritef(1, "file %s created!\n", curpath);
}

void usage(void) {
    fwritef(1, "usage: touch [-dFl] [file...]\n");
    exit();
}

void umain(int argc, char **argv) { // only has the simple tree, no -d, -F, -l
    int i;
    ARGBEGIN
    {
        default:
            usage();
        case 'd':
        case 'F':
        case 'l':
            flag[(u_char) ARGC()]++;
    }
    ARGEND
    if (argc == 0) return;
    else for (i = 0; i < argc; i++) touch(argv[i], argv[i]);
}