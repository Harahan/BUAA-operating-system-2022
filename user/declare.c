#include "lib.h"
#include "sh.h"
int flag[256];
void print_var() {
    writef(LIGHT_CYAN(--------environment variables--------\n));
    char *name_table[1<<8], *value_table[1<<8];
    syscall_env_var(name_table, value_table, 0, 0, 4);
    int i = 0;
    while(name_table[i]) {
        fwritef(1, GREEN(%s) " =%s\n", name_table[i], value_table[i]);
        i++;
    }
}

void declare(char* name, char* value, int vis, int readonly) {
    if (syscall_env_var(name, value, vis, readonly, 0) < 0) fwritef(1, "environment var" RED([%s]) "is readonly!\n", name);
}

void usage(void) {
    fwritef(1, "declare [-xr] [NAME =[VALUE]]");
}

void umain(int argc, char** argv) {
    int i;
    if (argc == 1) {
        print_var();
        return;
    }
    if (argc == 2) {
        declare(argv[1], "NULL", 1, 0);
        writef("finish declare " GREEN(%s)"\n", argv[1]);
        return;
    } else if (argc == 3) {
        if (strcmp(argv[1], "-r") == 0) declare(argv[2], "NULL", 1, 1);
        else if (strcmp(argv[1], "-x") == 0) declare(argv[2], "NULL", 0, 0);
        else if (strcmp(argv[1], "-xr") == 0 || strcmp(argv[1], "-rx") == 0) declare(argv[2], "NULL", 0, 1);
        else {
            declare(argv[1], argv[2] + 1, 1, 0);
            writef("finish declare " GREEN(%s)"\n", argv[1]);
            return;
        }
    } else if (argc == 4) {
        // writef("%s %s", argv[2], argv[3] + 1);
        if (strcmp(argv[1], "-xr") == 0 || strcmp(argv[1], "-rx") == 0) declare(argv[2], argv[3] + 1, 0, 1);
        else if (strcmp(argv[1], "-r") == 0) declare(argv[2], argv[3] + 1, 1, 1);
        else if (strcmp(argv[1], "-x") == 0) declare(argv[2], argv[3] + 1, 0, 0);
    }
    writef("finish declare " GREEN(%s)"\n", argv[2]);
    return;
}