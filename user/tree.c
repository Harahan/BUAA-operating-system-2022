#include "lib.h"
#include "sh.h"
int flag[256];
void printline(char r, int n, int ret) {
    int i = 0;
    for (; i < n; i++) fwritef(1, "%c", r);
    if (ret) fwritef(1, "\n");
}

void walk(char *path, int level, int rec) {
    int fd, n;
    struct File f, *dir;
    char new[MAXPATHLEN] = {0};
    if (rec == 0) return;
    if ((fd = open(path, O_RDONLY)) < 0) user_panic("open %s: %e", path, fd);
    while ((n = readn(fd, &f, sizeof f)) == sizeof f) { // get the file names in the same path
        if (f.f_name[0]) {
            dir = &f;
            printline(' ', level * 4, 0); // make output formatting
            fwritef(1, "|-- ");
            if (dir->f_type == FTYPE_REG) fwritef(1, "%s", dir->f_name);
            else fwritef(1, LIGHT_BLUE(%s), dir->f_name);
            fwritef(1, "\n");
            if (dir->f_type == FTYPE_DIR) { // go to the dir bellow the current path
                strcpy(new, path);
                strcat(new, "/");
                strcat(new, f.f_name);
                walk(new, level + 1, rec - 1);
            }
        }
    }
}

void tree_start(char *path, u_int recursive) {
    fwritef(1, GREEN(.)" "PURPLE(%s\n), path);
    walk(path, 0, recursive);
}

void tree(char *path, char *prefix) {
    int r, fd;
    struct Stat st;
    if ((r = stat(path, &st)) < 0) user_panic("stat %s: %e", path, r);
    if (st.st_isdir && !flag['d']) tree_start(path, -1);
    else tree_start(path, -1);
}

void usage(void) {
    fwritef(1, "usage: tree [-dFl] [file...]\n");
    exit();
}

void umain(int argc, char **argv) { // only has the simple tree, no -d, -F, -l
    int i;
    char curpath[MAXPATHLEN];
    ARGBEGIN
    {
        default: usage();
        case 'd':
        case 'F':
        case 'l':
            flag[(u_char) ARGC()]++;
    }
    ARGEND
    if (curpath_get(curpath) < 0) {
        fwritef(1, "cd can't get environment var [curpath]\n");
        return;
    }
    if (argc == 0) tree(curpath, "");
    else for (i = 0;i < argc; i++) tree(argv[i], argv[i]);
}