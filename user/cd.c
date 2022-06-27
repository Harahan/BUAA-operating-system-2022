#include "lib.h"
#include "sh.h"
int flag[256];
void usage(void) {
    fwritef(1, "usage: cd [dir]\n");
    exit();
}

void umain(int argc, char **argv) {
    int i, r;
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
    if (argc == 0) {
        fwritef(1, "cd: too few args");
        return;
    }
    if (strcmp(argv[0], ".") == 0) return;
    char path[256];
    if (strcmp(argv[0], "..") == 0) {
        curpath_get_parent(path);
        curpath_set(path);
        fwritef(1, "cd: %s\n", path);
        return;
    } else {
        if ((r = curpath_get(path)) < 0) {
            fwritef(1, "cd can't get environment var [curpath]\n");
            return;
        }
        strcat(path, argv[0]);
        int len = strlen(path);
        if (path[len - 1] != '/') strcat(path, "/");
        struct Stat st;
        r = stat(path, &st);
        if (r == -E_NOT_FOUND) fwritef(1, "cd: " RED([%s]) " not found\n", path);
        else if(r < 0) fwritef(1, "cd: can't cd " RED([%s])"\n", path);
        else if (!st.st_isdir) fwritef(1, "cd: "RED(%s)" is not directory\n", path);
        else {
            if ((r = curpath_set(path)) < 0) fwritef(1, "cd: environment var can't be set to "RED(%s)"\n", path);
            fwritef(1, "curpath: %s\n", path);
        }
    }
}