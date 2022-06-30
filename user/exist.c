#include "lib.h"
#include "sh.h"

void umain(int argc, char** argv) {
    int shell_id = syscall_env_get_shell(0);
    // syscall_env_destroy(shell_id);
    writef("\n"LIGHT_BLUE(--------shell)" %d "LIGHT_BLUE(exist--------)"\n", shell_id);
}