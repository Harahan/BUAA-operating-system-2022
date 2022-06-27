#include "lib.h"
void clear() {
    writef("\x1b[2J\x1b[H");
}

void usage(void) {
    fwritef(1, "usage: clear\n");
}

void umain(int argc, char **argv) {
    clear();
}