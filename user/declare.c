#include "lib.h"
#include "sh.h"
int flag[256];
void print_var() {
    writef(LIGHT_PURPLE(--------environment variables--------\n));
    char *name_table[1<<8], *value_table[1<<8];
    syscall_env_var(name_table, value_table, 0, 0, 4);
    int i = 0;
    while(name_table[i]) {
        fwritef(1, GREEN(%s) " =%s\n", name_table[i], value_table[i]);
        i++;
    }
}

void declare(char* name, char* value, int vis, int readonly) {
    if (syscall_env_var(name, value, vis, readonly, 0) < 0) {
        fwritef(1, "environment var" RED([%s]) "is readonly!\n", name);
        exit();
    }
}

void usage(void) {
    fwritef(1, "declare [-xr] [NAME =[VALUE]]");
}

void umain(int argc, char** argv) {
    int i, readonly = 0, vis = 1;
    ARGBEGIN
    {
        case 'r':
            readonly = 1;
            break;
        case 'x':
            vis = 0;
            break;
        case 'xr':
        case 'rx':
            readonly = 1; vis = 0;
            break;
        default:
            usage();
    }
    ARGEND
    if (argc == 0) {
        print_var();
        return;
    }
    if (argc == 1) declare(argv[0], "", vis, readonly);
    else if (argc == 2) declare(argv[0], argv[1] + 1, vis, readonly);
    else usage();
    writef("finish declare " GREEN(%s)"\n", argv[0]);
    return;
}