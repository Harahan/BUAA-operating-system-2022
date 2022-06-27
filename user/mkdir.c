#include "lib.h"
int flag[256];
void mkdir(char *path, char *prefix) {
    int r, fd;
    struct Stat st;
    char curpath[MAXPATHLEN] = {'\0'};
    if ((r = curpath_get(curpath)) < 0) fwritef(1, "mkdir: can't get environment var [curpath]\n");
    if (path[0] == '/') strcpy(curpath, path);
    else {
        if (curpath[strlen(curpath) - 1] != '/') strcat(curpath, "/");
        strcat(curpath, path);
    }
    if ((r = create(curpath, FTYPE_DIR)) < 0) { // use fsipc to create a file, and which is created the path if the path is not exist
        fwritef(1, "directory %s already exists!\n", curpath);
        return;
    }
    fwritef(1, "created directory %s!\n", curpath);
}

void usage(void) {
    fwritef(1, "usage: mkdir [-dFl] [file...]\n");
    exit();
}

void umain(int argc, char **argv) { // only has the simple tree, no -d, -F, -l
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
    else for (i = 0;i < argc; i++) mkdir(argv[i], argv[i]);
}

