#include "lib.h"
void history_init() {
    int r;
    if ((r = create("/.history", FTYPE_REG)) < 0) user_panic("init .history failed: %d.\n", r);
}

void history_save(char *s) {
    int r;
    if ((r = open("/.history", O_RDWR | O_CREAT | O_APP)) < 0) user_panic("open .history failed");
    fwritef(r, s); // '\0' doesn't write to .history
    fwritef(r, "\n");
    close(r);
}

int history_read(char (*cmd)[128]) { // used char cmd[][] to save the cmd in .history
    int r, fd, cur = 1, i = 0, cmdi;
    char buf[128 * 128];
    if ((fd = open("/.history", O_RDONLY)) < 0) user_panic("open .history failed");
    if ((r = read(fd, buf, (long) sizeof buf)) < 0) user_panic("read .history failed");
    close(fd);
    while(buf[i]) {
        int cmdj = 0;
        while(buf[i] && buf[i] != '\n') cmd[cmdi][cmdj++] = buf[i++];
        if (!buf[i]) break;
        ++i; ++cmdi;
    }
    return cmdi;
}