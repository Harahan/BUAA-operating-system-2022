# Exam

ide.c

```c
int time_read() {
    int addr = 0x15000000, offset = 0x0010, time = 1;
    syscall_write_dev(&time, addr, 4);
    syscall_read_dev(&time, addr + offset, 4);
    return time;
}

void raid0_write(u_int secno, void *src, u_int nsecs) {
    int i = 0;
    while (i < nsecs) {
        if ((secno + i) % 2) ide_write(2, (secno + i) / 2, src + i * 0x200, 1);
        else ide_write(1, (secno + i) / 2, src + i * 0x200, 1);
        i++;
    }
}

void raid0_read(u_int secno, void *dst, u_int nsecs) {
    int i = 0;
    while (i < nsecs) {
        if ((secno + i) % 2) ide_read(2, (secno + i) / 2, dst + i * 0x200, 1);
        else ide_read(1, (secno + i) / 2, dst + i * 0x200, 1);
        i++;
    }
}
```

fs.h

```c
int time_read();
void raid0_write(u_int secno, void *src, u_int nsecs);
void raid0_read(u_int secno, void *dst, u_int nsecs);
```

# Extra

ide.c

```c
int raid4_valid(u_int diskno) {
    int offset = 0, op_status1 = 0, read = 0;
    if (syscall_write_dev(&diskno, 0x13000010, 4) != 0) user_panic("write_failed!\n");
    if (syscall_write_dev(&offset, 0x13000000, 4) != 0) user_panic("write_failed!\n");
    if (syscall_write_dev(&read, 0x13000020, 4) != 0) user_panic("write_failed!\n");
    if (syscall_read_dev(&op_status1, 0x13000030, 4) != 0) user_panic("read_failed!\n");
    return op_status1;
}

int _get_fault_disks_sum() {
    int i = 1, ans = 0;
    for (;i <= 5; i++) if (raid4_valid(i) == 0) ans++;
    return ans;
}

int raid4_write(u_int blockno, void *src) {
    char check[BY2PG / 8];
    int i = 0, j = -1, secn = 1;
    for (;secn <= 4; secn++) {
        if (raid4_valid(secn) != 0) {
            ide_write(secn, 2 * blockno, src + BY2PG * (secn - 1) / 8, 1);
            ide_write(secn, 2 * blockno+1, src + BY2PG * (secn + 3) / 8, 1);
        }
    }
    if (raid4_valid(5) != 0) {
        while (j++ < 1) {
            user_bzero(check1, BY2PG);
            for (; i < BY2PG / 8; i++)
                check[i] = ((char *)(src + (4 * j) * BY2PG / 8))[i] ^ \
                ((char *)(src + (4 * j + 1) * BY2PG / 8))[i] ^ ((char *)(src + (4 * j + 2) * BY2PG / 8))[i] ^ \
                ((char *)(src + (4 * j + 3) * BY2PG / 8))[i];
            ide_write(5, 2*blockno + j, check, 1);
        }
    }
    return _get_fault_disks_sum();
}

int raid4_read(u_int blockno, void *dst) {
    char check1[BY2PG / 8], check2[BY2PG / 8], raid_check1[BY2PG / 8], raid_check2[BY2PG / 8];
    int fault_disk = _get_fault_disks_sum(), i, j, k = 1, secn = 1, ok = 0;
    user_bzero(check1, BY2PG); user_bzero(check2, BY2PG);
    user_bzero(raid_check1, BY2PG); user_bzero(raid_check2, BY2PG);
    if (fault_disk > 1) return fault_disk;
    for (; secn <= 4; secn++) {
        if (raid4_valid(secn) != 0) {
            ide_read(secn, 2 * blockno, dst + BY2PG * (secn - 1) / 8, 1);
            ide_read(secn, 2 * blockno + 1, dst + BY2PG * (secn + 3) / 8, 1);
        }
    }
    if (raid4_valid(5) == 0) return 1;
    ide_read(5, 2*blockno, raid_check1, 1); ide_read(5, 2*blockno+1, raid_check2, 1);
    if (fault_disk == 0) {
        for (i = 0; i < BY2PG / 8; i++) {
            check1[i] = ((char *)dst)[i] ^ ((char *)(dst + BY2PG / 8))[i] ^ \
                ((char *)(dst + 2 * BY2PG / 8))[i] ^ ((char *)(dst + 3 * BY2PG / 8))[i];
            if (raid_check1[i] != check1[i]) {ok = -1; break;}
        }
        for (i = 0; i < BY2PG / 8; i++) {
            check2[i] = ((char *)(dst + 4 * BY2PG / 8))[i] ^ ((char *)(dst + 5 * BY2PG / 8))[i] ^ \
                ((char *)(dst + 6 * BY2PG / 8))[i] ^ ((char *)(dst + 7 * BY2PG / 8))[i];
            if (raid_check2[i] != check2[i]) {ok = -1; break;}
        }
        return ok; 
    }
    for (; k <= 4; k++) {
        if (raid4_valid(k) == 0) {
            for (i = 0; i < BY2PG / 8; i++) {
                char restore_data1 = 0, restore_data2 = 0;
                for (j = 1; j <= 5; j++) if (k != j) {
                        if (j != 5) restore_data1 ^= ((char *)(dst + (j - 1) * BY2PG / 8))[i], \
                                restore_data2 ^= ((char *)(dst + (j + 3) * BY2PG / 8))[i];
                        else restore_data1 ^= raid_check1[i], restore_data2 ^= raid_check2[i];
                    }
                ((char *)(dst + (k - 1) * BY2PG / 8))[i] = restore_data1;
                ((char *)(dst + (k + 3) * BY2PG / 8))[i] = restore_data2;
            }
        }
    }
    return 1;
}
```

fs.h

```c
int raid4_valid(u_int diskno);
int raid4_write(u_int blockno, void *src);
int raid4_read(u_int blockno, void *dst);
```
